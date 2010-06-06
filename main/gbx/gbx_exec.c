/***************************************************************************

  gbx_exec.c

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __GBX_EXEC_C

#include "gb_common.h"
#include "gb_error.h"
#include "gbx_type.h"

#include <unistd.h>
#include <sys/time.h>

#include "gb_limit.h"
#include "gbx_subr.h"
#include "gbx_stack.h"
#include "gbx_debug.h"

#include "gbx_string.h"
#include "gbx_date.h"

#include "gbx_c_collection.h"

#include "gbx_api.h"
#include "gbx_exec.h"

//#define DEBUG_STACK 1

/* Current virtual machine state */
STACK_CONTEXT EXEC_current = { 0 };
/* Stack pointer */
VALUE *SP = NULL;
/* Temporary storage or return value of a native function */
VALUE TEMP;
/* Return value of a gambas function */
VALUE RET;
/* SUPER was used for this stack pointer */
VALUE *EXEC_super = NULL;
/* CPU endianness */
bool EXEC_big_endian;
/* Current iterator */
CENUM *EXEC_enum;

bool EXEC_debug = FALSE; // debugging mode
bool EXEC_arch = FALSE; // executing an archive
bool EXEC_fifo = FALSE; // debugging through a fifo
const char *EXEC_fifo_name = NULL; // fifo name
bool EXEC_keep_library = FALSE; // do not unload libraries

EXEC_HOOK EXEC_Hook = { NULL };
EXEC_FUNCTION EXEC;
bool EXEC_main_hook_done = FALSE;
int EXEC_return_value = 0;
bool EXEC_got_error = FALSE;
uint64_t EXEC_byref = 0;

const char EXEC_should_borrow[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0 };

void EXEC_init(void)
{
	union {
		char _string[4];
		uint _int;
		}
	test;

	PC = NULL;
	BP = NULL;
	OP = NULL;
	CP = NULL;
	RP->type = T_VOID;

	test._string[0] = 0xAA;
	test._string[1] = 0xBB;
	test._string[2] = 0xCC;
	test._string[3] = 0xDD;

	EXEC_big_endian = test._int == 0xAABBCCDDL;
	
	if (EXEC_big_endian)
		fprintf(stderr, "** WARNING: CPU is big endian\n");

	DATE_init();
}


void EXEC_borrow(TYPE type, VALUE *value)
{
	static const void *jump[16] = {
		&&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE,
		&&__STRING, &&__NONE, &&__NONE, &&__VARIANT, &&__FUNCTION, &&__NONE, &&__NONE
		};

	goto *jump[type];

__VARIANT:
	if (value->_variant.vtype == T_STRING)
		STRING_ref(value->_variant.value._string);
	else if (TYPE_is_object(value->_variant.vtype))
		OBJECT_REF(value->_variant.value._object, "BORROW");
	return;

__FUNCTION:
	OBJECT_REF(value->_function.object, "BORROW");
	return;

__STRING:
	STRING_ref(value->_string.addr);

__NONE:
	return;
}


void UNBORROW(VALUE *value)
{
	static const void *jump[16] = {
		&&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE,
		&&__STRING, &&__NONE, &&__NONE, &&__VARIANT, &&__FUNCTION, &&__NONE, &&__NONE
		};

	TYPE type = value->type;

	if (UNLIKELY(TYPE_is_object(type)))
	{
		OBJECT_UNREF_KEEP(value->_object.object, "UNBORROW");
		return;
	}
	
	goto *jump[type];

__VARIANT:
	if (value->_variant.vtype == T_STRING)
		STRING_unref_keep(&value->_variant.value._string);
	else if (TYPE_is_object(value->_variant.vtype))
		OBJECT_UNREF_KEEP(value->_variant.value._object, "UNBORROW");
	return;

__FUNCTION:
	OBJECT_UNREF_KEEP(value->_function.object, "UNBORROW");
	return;

__STRING:
	STRING_unref_keep(&value->_string.addr);

__NONE:
	return;
}


void EXEC_release(TYPE type, VALUE *value)
{
	static const void *jump[16] = {
		&&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE,
		&&__STRING, &&__NONE, &&__NONE, &&__VARIANT, &&__FUNCTION, &&__NONE, &&__NONE
		};

	goto *jump[type];

__VARIANT:
	if (value->_variant.vtype == T_STRING)
		STRING_unref(&value->_variant.value._string);
	else if (TYPE_is_object(value->_variant.vtype))
		OBJECT_UNREF(value->_variant.value._object, "RELEASE");
	return;

__FUNCTION:
	OBJECT_UNREF(value->_function.object, "RELEASE");
	return;

__STRING:
	STRING_unref(&value->_string.addr);

__NONE:
	return;
}

void RELEASE_many(VALUE *value, int n)
{
	static const void *jump[16] = {
		&&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE,
		&&__STRING, &&__NONE, &&__NONE, &&__VARIANT, &&__FUNCTION, &&__NONE, &&__NONE
		};

	TYPE type;
	
	while (n)
	{
		n--;
		value--;
		
		type = value->type;
	
		if (UNLIKELY(TYPE_is_object(type)))
		{
			OBJECT_UNREF(value->_object.object, "RELEASE");
			continue;
		}
		
		goto *jump[type];
	
	__VARIANT:
		if (value->_variant.vtype == T_STRING)
			STRING_unref(&value->_variant.value._string);
		else if (TYPE_is_object(value->_variant.vtype))
			OBJECT_UNREF(value->_variant.value._object, "RELEASE");
		continue;
	
	__FUNCTION:
		OBJECT_UNREF(value->_function.object, "RELEASE");
		continue;
	
	__STRING:
		STRING_unref(&value->_string.addr);
	
	__NONE:
		continue;
	}
}

#if 0
void DUMP(VALUE *value)
{
	static void *jump[16] = {
		&&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE, &&__NONE,
		&&__STRING, &&__NONE, &&__VARIANT, &&__ARRAY, &&__NONE, &&__FUNCTION, &&__NONE, &&__NONE
		};

	TYPE type = value->type;

	printf("type = %p / ", (void *)type);

	if (TYPE_is_object(type))
		goto __OBJECT;
	else
		goto *jump[type];

__STRING:
	printf("STRING %p\n", value->_string.addr);
	return;

__OBJECT:
	if (value->_object.object)
	{
		printf("OBJECT (%p)\n", value->_object.object);
		printf("-> %s\n", OBJECT_class(value->_object.object)->name);
	}
	else
		printf("OBJECT (NULL)\n");
	return;

__VARIANT:
	if (value->_variant.vtype == T_STRING)
		printf("STRING %p\n", *((char **)value->_variant.value));
	else if (TYPE_is_object(value->_variant.vtype))
		printf("OBJECT (%s %p)\n", OBJECT_class(*((void **)value->_variant.value))->name, *((void **)value->_variant.value));
	return;

__FUNCTION:
	printf("FUNCTION %s (%s %p)\n", value->_function.class->name, OBJECT_class(value->_function.object)->name, value->_function.object);
	return;

__ARRAY:
	printf("ARRAY\n");
	return;

__NONE:
	printf("\n");
	return;
}
#endif

void EXEC_release_return_value(void)
{
	RELEASE(RP);
	RP->type = T_VOID;
}


#define print_register() \
	printf("| SP = %d  BP = %d  FP = %p  PC = %p  EC = %p\n", SP - (VALUE *)STACK_base, BP - (VALUE *)STACK_base, FP, PC, EC)

static bool exec_enter_can_quick(void)
{
	int i;
	FUNCTION *func;
	int nparam = EXEC.nparam;

	func = &EXEC.class->load->func[EXEC.index];

	/* check number of arguments */

	if (func->npmin < func->n_param || nparam != func->n_param || func->vararg)
		return FALSE;

	/* check arguments type */

	for (i = 0; i < nparam; i++)
	{
		if (SP[i - nparam].type != func->param[i].type)
			return FALSE;
	}

	return TRUE;
}

static void init_local_var(CLASS *class, FUNCTION *func)
{
	static const void *jump[] = {
		&&__VOID, &&__BOOLEAN, &&__BYTE, &&__SHORT, &&__INTEGER, &&__LONG, &&__SINGLE, &&__FLOAT, 
		&&__DATE, &&__STRING, &&__STRING, &&__POINTER, &&__VARIANT, &&__FUNCTION, &&__CLASS, &&__NULL, 
		&&__OBJECT
		};
		
	CLASS_LOCAL *local;
	int n;
	CTYPE ctype;
	VALUE *value;
	
	local = func->local;
	value = SP;
	
	for (n = func->n_local; n; n--, value++, local++)
	{
		ctype = local->type;
		
		value->type = ctype.id;
		goto *jump[ctype.id];

	__BOOLEAN:
	__BYTE:
	__SHORT:
	__INTEGER:

		value->_integer.value = 0;
		continue;

	__LONG:

		value->_long.value = 0;
		continue;

	__SINGLE:
	__FLOAT:
		value->_float.value = 0;
		continue;

	__STRING:
		value->_string.addr = NULL;
		value->_string.start = 0;
		value->_string.len = 0;
		continue;

	__VARIANT:
		value->_variant.vtype = T_NULL;
		continue;

	__POINTER:
		value->_pointer.value = NULL;
		continue;

	__DATE:
		value->_date.date = 0;
		value->_date.time = 0;
		continue;

	__VOID:
		continue;

	__OBJECT:
		if (ctype.value >= 0)
			value->type = (TYPE)class->load->class_ref[ctype.value];
		value->_object.object = NULL;
		continue;

	__FUNCTION:
	__CLASS:
	__NULL:
		ERROR_panic("VALUE_default: Unknown default type");
	}
	
	SP = value;
}

// EXEC.nparam must be set to the amount of stack that must be freed if an exception is raised during EXEC_enter()

void EXEC_enter(void)
{
	int i;
	FUNCTION *func; // = EXEC.func;
	int nparam = EXEC.nparam;
	void *object = EXEC.object;
	CLASS *class = EXEC.class;

	#if DEBUG_STACK
	printf("\n| >> EXEC_enter(%s, %ld, %d)\n", EXEC.class->name, EXEC.index, nparam);
	print_register();
	#endif

	/*
	func_id = value->index;

	if (value->kind & FUNCTION_PUBLIC)
		func_id = (int)(class->table[func_id].desc.method->exec;

	*/

	func = &class->load->func[EXEC.index];
	#if DEBUG_STACK
	if (func->debug)
		printf(" | >> %s\n", func->debug->name);
	#endif

	/* check number of arguments */

	if (UNLIKELY(nparam < func->npmin))
		THROW(E_NEPARAM);
	else if (UNLIKELY(nparam > func->n_param && !func->vararg))
		THROW(E_TMPARAM);

	/* mandatory arguments */

	for (i = 0; i < func->npmin; i++)
	{
		VALUE_conv(SP - nparam + i, func->param[i].type);
		/*BORROW(SP - nparam + i);*/
	}

	if (UNLIKELY(func->npmin < func->n_param))
	{
		/* optional arguments */

		for (i = func->npmin; i < nparam; i++)
		{
			if (SP[- nparam + i].type == T_VOID)
				SP[- nparam + i]._void.ptype = func->param[i].type;
			else
			{
				VALUE_conv(SP - nparam + i, func->param[i].type);
				/*BORROW(SP - nparam + i);*/
			}
		}

		/* missing optional arguments */

		if (UNLIKELY(nparam < func->n_param))
		{
			STACK_check(func->n_param - nparam);

			for (i = nparam; i < func->n_param; i++)
			{
				SP->type = T_VOID;
				SP->_void.ptype = func->param[i].type;
				SP++;
			}
			
			EXEC.nparam = func->n_param;
		}
	}

	/* save context & check stack*/

	STACK_push_frame(&EXEC_current, func->stack_usage);

	/* enter function */

	BP = SP;
	if (UNLIKELY(func->vararg))
		PP = SP - (nparam - func->n_param);
	else
		PP = SP;
	FP = func;
	PC = func->code;
	OP = object;
	CP = class;
	//AP = ARCH_from_class(CP);
	EP = NULL;

	if (UNLIKELY(func->error))
	{
		#if DEBUG_ERROR
			printf("EXEC_enter: EC = PC + %d\n", func->error);
		#endif
		EC = PC + func->error;
	}
	else
		EC = NULL;

	/* reference the object so that it is not destroyed during the function call */
	OBJECT_REF(OP, "EXEC_enter");

	/*printf("PC = %p  nparam = %d\n", PC, FP->n_param);*/

	/* local variables initialization */

	if (LIKELY(func->n_local > 0))
		init_local_var(class, func);

	/* control variables initialization */

	if (LIKELY(func->n_ctrl > 0))
	{
		for (i = 0; i < func->n_ctrl; i++)
		{
			SP->type = T_VOID;
			SP++;
		}
	}

	/*printf("EXEC_enter: nparam = %d  nlocal = %d  nctrl = %d\n", func->n_param, func->n_local, func->n_ctrl);*/

	RP->type = T_VOID;

	#if DEBUG_STACK
	printf("| << EXEC_enter()\n");
	print_register();
	#endif
}


void EXEC_enter_check(bool defined)
{
	if (defined && exec_enter_can_quick())
	{
		*PC = (*PC & 0xFF) | C_CALL_QUICK;
		EXEC_enter_quick();
	}
	else
	{
		*PC = (*PC & 0xFF) | C_CALL_NORM;
		EXEC_enter();
	}
}


void EXEC_enter_quick(void)
{
	int i;
	FUNCTION *func;;
	void *object = EXEC.object;
	CLASS *class = EXEC.class;

	#if DEBUG_STACK
	printf("\n| >> EXEC_enter_quick(%s, %ld, %d)\n", EXEC.class->name, EXEC.index, EXEC.nparam);
	print_register();
	#endif

	func = &class->load->func[EXEC.index];
	#if DEBUG_STACK
	if (func->debug)
		printf(" | >> %s\n", func->debug->name);
	#endif

	/* save context & check stack */

	STACK_push_frame(&EXEC_current, func->stack_usage);

	/* enter function */

	BP = SP;
	PP = SP;
	FP = func;
	PC = func->code;
	OP = object;
	CP = class;
	EP = NULL;

	if (UNLIKELY(func->error))
		EC = PC + func->error;
	else
		EC = NULL;

	/* reference the object so that it is not destroyed during the function call */
	OBJECT_REF(OP, "EXEC_enter_quick");

	/* local variables initialization */

	if (LIKELY(func->n_local != 0))
		init_local_var(class, func);

	/* control variables initialization */

	if (LIKELY(func->n_ctrl != 0))
	{
		for (i = 0; i < func->n_ctrl; i++)
		{
			SP->type = T_VOID;
			SP++;
		}
	}

	RP->type = T_VOID;

	#if DEBUG_STACK
	printf("| << EXEC_enter()\n");
	print_register();
	#endif
}


void EXEC_leave(bool drop)
{
	int n, nb, i, nparam;
	bool keep_ret_value = FALSE;
	VALUE ret;
	ushort *pc;
	VALUE *xp, *pp;
	int bit, nbyref;
	ushort *pc_func;
	int nbyref_func;

#if DEBUG_STACK
	printf("| >> EXEC_leave\n");
	print_register();
#endif

	/* Save the return value. It can be erased by OBJECT_UNREF() */

	//ret = *RP;
	VALUE_copy(&ret, RP);
	
  pc = STACK_get_previous_pc();
  nb = 0;
  nparam = FP->n_param;
  
	/* ByRef arguments management */

	if (LIKELY(!(pc && PCODE_is(pc[1], C_BYREF))))
		goto __LEAVE_NORMAL;
		
	pc++;
	nbyref = 1 + (*pc & 0xF);
	pc_func = FP->code;
	
	if (LIKELY(!PCODE_is(*pc_func, C_BYREF)))
		goto __LEAVE_NORMAL;
	
	nbyref_func = 1 + (*pc_func & 0xF);
	if (nbyref_func < nbyref)
		goto __LEAVE_NORMAL;
		
	for (i = 1; i <= nbyref; i++)
	{
		if (pc[i] & ~pc_func[i])
			goto __LEAVE_NORMAL;
	}
	
	xp = PP - nparam;
	pp = xp;
	
	for (i = 0, n = 0; i < nparam; i++)
	{
		bit = i & 15;
		if (bit == 0)
			n++;
		
		if (n <= nbyref && (pc[n] & (1 << bit)))
		{
			//printf("pp[%d] -> pp[%d]\n", i, nb);
			xp[nb] = *pp;
			nb++;
		}
		else
		{
			//printf("pp[%d] release (%d)\n", i, pp->type);
			RELEASE(pp);
		}
		pp++;
	}		

	pc--;
	n = SP - PP;
	#if DEBUG_STACK
	printf("release = %d, nparam = %d, byref = %d\n", n, FP->n_param, nb);
	#endif
	RELEASE_MANY(SP, n);
	SP -= nparam;
	SP += nb;
	OBJECT_UNREF(OP, "EXEC_leave");
	SP -= nb;
	STACK_pop_frame(&EXEC_current);
	PC += nbyref + 1;
	goto __RETURN_VALUE;

__LEAVE_NORMAL:  	
	
	n = nparam + (SP - PP);
	RELEASE_MANY(SP, n);

	OBJECT_UNREF(OP, "EXEC_leave");
	STACK_pop_frame(&EXEC_current);

__RETURN_VALUE:

	if (pc && !drop)
	{
		drop = PCODE_is_void(*pc);
		if (SP[-1].type == T_FUNCTION)
		{
			SP--;
			OBJECT_UNREF(SP->_function.object, "EXEC_leave");
		}
	}
	else
	{
		drop = TRUE;
		keep_ret_value = TRUE;
	}

	if (!drop)
	{
		//*SP = ret;
		COPY_VALUE(SP, &ret);
		RP->type = T_VOID;
		if (PCODE_is_variant(*PC))
			VALUE_conv_variant(SP);
		SP++;
	}
	else if (!keep_ret_value)
		EXEC_release_return_value();

	SP += nb;
	
#if DEBUG_STACK
	printf("| << EXEC_leave()\n");
	print_register();
	printf("\n");
#endif
	return;
}


void EXEC_function_real()
{
	// We need to push a void frame, because EXEC_leave looks at *PC to know if a return value is expected
	STACK_push_frame(&EXEC_current, 0);

	PC = NULL;

	TRY
	{
		EXEC_enter();
	}
	CATCH
	{
		RELEASE_MANY(SP, EXEC.nparam);
		STACK_pop_frame(&EXEC_current);
		PROPAGATE();	
	}
	END_TRY
	
	EXEC_function_loop();
}

void EXEC_function_loop()
{
	bool retry = FALSE;

	if (LIKELY(PC != NULL))
	{
		do
		{
			TRY
			{
				EXEC_loop();
				retry = FALSE;
			}
			CATCH
			{
				// QUIT was called
				if (ERROR->info.code == E_ABORT)
				{
					#if DEBUG_ERROR
					fprintf(stderr, "#0 QUIT\n");
					#endif
					ERROR_lock();
					while (PC != NULL)
						EXEC_leave(TRUE);
					ERROR_unlock();

					//STACK_pop_frame(&EXEC_current);
					PROPAGATE();
				}
				// We are in a TRY
				else if (EP != NULL)
				{
					#if DEBUG_ERROR
					fprintf(stderr, "#1 EP = %d  SP = %d\n", EP - (VALUE *)STACK_base, SP - (VALUE *)STACK_base);
					fprintf(stderr, "TRY\n");
					#endif
					ERROR_set_last();
					
					while (SP > EP)
						POP();

					PC = EC;
					EP = NULL;
					retry = TRUE;
					/* On va directement sur le END TRY */
				}
				// There is a CATCH in the function
				else if (EC != NULL)
				{
					#if DEBUG_ERROR
					fprintf(stderr, "#2 EC = %p\n", EC);
					fprintf(stderr, "CATCH\n");
					#endif

					ERROR_set_last();
					PC = EC;
					EC = NULL;
					retry = TRUE;
				}
				// There is no event handler in the function
				else
				{
					#if DEBUG_ERROR
					fprintf(stderr, "#3\n");
					fprintf(stderr, "NOTHING\n");
					#endif
					//ERROR_INFO save = { 0 };
					//ERROR_save(&save);

					ERROR_set_last();

					if (EXEC_debug && !STACK_has_error_handler())
					{
						if (TP && TC)
						{
							ERROR_lock();
							while (BP > TP)
							{
								EXEC_leave(TRUE);
								if (!PC)
									STACK_pop_frame(&EXEC_current);
							}
							while (SP > TP)
								POP();
							PC = TC;
							ERROR_unlock();
						}

						DEBUG.Main(TRUE);
						retry = TRUE;
					}
					else
					{
						ERROR_lock();
						while (PC != NULL && EC == NULL)
							EXEC_leave(TRUE);
						ERROR_unlock();

						if (PC == NULL)
						{
							/*printf("try to propagate\n");*/
							STACK_pop_frame(&EXEC_current);
							
							//ERROR_restore(&save);
							//ERROR_set_last();
							PROPAGATE();
						}

						if (EP != NULL)
						{
							#if DEBUG_ERROR
							fprintf(stderr, "#4 EP = %d  SP = %d\n", EP - (VALUE *)STACK_base, SP - (VALUE *)STACK_base);
							#endif

							ERROR_lock();
							while (SP > EP)
								POP();
							ERROR_unlock();

							EP = NULL;
							/* On va directement sur le END TRY */
						}

						PC = EC;
						EC = NULL;

						retry = TRUE;
					}
					
					//ERROR_restore(&save);
					//ERROR_set_last();
				}

				while (SP < EXEC_super)
					EXEC_super = ((VALUE *)EXEC_super)->_object.super;
			}
			END_TRY

			#if DEBUG_ERROR
			if (retry)
				fprintf(stderr, "RETRY %p\n", PC);
			#endif
		}
		while (retry);
	}

	STACK_pop_frame(&EXEC_current);
}


static bool exec_native_can_quick(void)
{
	CLASS_DESC_METHOD *desc = EXEC.desc;
	int i;
	int nparam = EXEC.nparam;
	
	/* check number of arguments */

	//fprintf(stderr, "exec_native_can_quick: %s.%s(%d) npmin:%d npmax:%d npvar:%d\n", desc->class->name, desc->name, nparam, desc->npmin, desc->npmax, desc->npvar);

	if (desc->npmin < desc->npmax || nparam != desc->npmin || desc->npvar)
		return FALSE;
		
	/* check arguments type */

	for (i = 0; i < nparam; i++)
	{
		if (SP[i - nparam].type != desc->signature[i])
			return FALSE;
	}

	return TRUE;
}

void EXEC_native_check(bool defined)
{
	if (defined && exec_native_can_quick())
	{
		*PC = (*PC & 0xFF) | C_CALL_QUICK;
		EXEC_native_quick();
	}
	else
	{
		*PC = (*PC & 0xFF) | C_CALL_NORM;
		EXEC_native();
	}

}

#define EXEC_call_native_inline(_exec, _object, _type, _param) \
({ \
	GAMBAS_Error = FALSE; \
	\
	(*(_exec))((_object), (void *)(_param)); \
	\
	if (UNLIKELY(GAMBAS_Error)) \
	{ \
		GAMBAS_Error = FALSE; \
		error = TRUE; \
	} \
	else \
	{ \
		TYPE __type = (_type); \
		if (UNLIKELY(TYPE_is_pure_object(_type) && TEMP.type != T_NULL && TEMP.type != T_VOID)) \
		{ \
			if (UNLIKELY(((CLASS *)__type)->override != NULL)) \
				__type = (TYPE)(((CLASS *)__type)->override); \
			\
			if (TEMP.type == T_CLASS) \
				TEMP._class.class = (CLASS *)__type; \
			else \
				TEMP.type = __type; \
			\
			error = FALSE; \
		} \
		else \
			error = FALSE; \
	} \
})

bool EXEC_call_native(void (*exec)(), void *object, TYPE type, VALUE *param)
{
	bool error;
	
	EXEC_call_native_inline(exec, object, type, param);
	return error;
}

void EXEC_native_quick(void)
{
	CLASS_DESC_METHOD *desc = EXEC.desc;
	bool drop = EXEC.drop;
	int nparam = EXEC.nparam;
	//bool use_stack = EXEC.use_stack; // Always TRUE
	void *object = EXEC.object;

	bool error;
	void *free_later;
	VALUE ret;

	EXEC_call_native_inline(desc->exec, object, desc->type, &SP[-nparam]);
	COPY_VALUE(&ret, &TEMP);

	RELEASE_MANY(SP, nparam);
	
	if (UNLIKELY(error))
	{
		POP();
		PROPAGATE();
	}
	
	/* Si la description de la fonction se trouve sur la pile */

	SP--;
	free_later = SP->_function.object;
	SP->type = T_NULL;

	if (desc->type == T_VOID)
	{
		if (!drop)
		{
			SP->type = T_VOID;
			SP->_void.ptype = T_NULL;
			SP++;
		}
	}
	else
	{
		BORROW(&ret);

		if (drop)
		{
			RELEASE(&ret);
		}
		else
		{
				
			//*SP = ret;
			COPY_VALUE(SP, &ret);
			SP++;
		}
	}

	OBJECT_UNREF(free_later, "EXEC_native (FUNCTION)");

	#if DEBUG_STACK
	printf("| << EXEC_native: %s (%p)\n", desc->name, &desc);
	#endif
}


void EXEC_native(void)
{
	CLASS_DESC_METHOD *desc = EXEC.desc;
	bool drop = EXEC.drop;
	int nparam = EXEC.nparam;
	bool use_stack = EXEC.use_stack;
	void *object = EXEC.object;

	int i; /* ,j */
	VALUE *value;
	TYPE *sign;
	bool error;
	void * NO_WARNING(free_later);
	int n, nm;
	VALUE ret;

	#if DEBUG_STACK
	printf("| >> EXEC_native: %s.%s (%p)\n", EXEC.class->name, desc->name, &desc);
	#endif

	//printf("EXEC_native: nparam = %d desc->npvar = %d\n", nparam, desc->npvar);

	TRY
	{
		n = desc->npmin;
		nm = desc->npmax;
		
		if (UNLIKELY(nparam < n))
			THROW(E_NEPARAM);

		if (LIKELY(!desc->npvar))
		{
			if (UNLIKELY(nparam > nm))
				THROW(E_TMPARAM);

			value = &SP[-nparam];
			sign = desc->signature;

			for (i = 0; i < n; i++, value++, sign++)
				VALUE_conv(value, *sign);

			if (UNLIKELY(n < nm))
			{
				for (; i < nparam; i++, value++, sign++)
				{
					if (value->type != T_VOID)
						VALUE_conv(value, *sign);
				}

				n = nm - nparam;

				if (UNLIKELY(STACK_check(n)))
				{
					STACK_RELOCATE(value);
				}
				
				SP += n;
				nparam = nm;

				for (; i < nparam; i++, value++)
					value->type = T_VOID;
			}
		}
		else
		{
			value = &SP[-nparam];
			sign = desc->signature;

			for (i = 0; i < n; i++, value++, sign++)
				VALUE_conv(value, *sign);

			nm = desc->npmax;
			if (UNLIKELY(n < nm))
			{
				if (UNLIKELY(nparam < nm))
				{
					for (; i < nparam; i++, value++, sign++)
					{
						if (value->type != T_VOID)
							VALUE_conv(value, *sign);
					}

					n = nm - nparam;

					if (UNLIKELY(STACK_check(n)))
					{
						STACK_RELOCATE(value);
					}
					
					SP += n;
					nparam = nm;

					for (; i < nparam; i++, value++)
						value->type = T_VOID;
				}
				else
				{
					n = desc->npmax;
					for (; i < n; i++, value++, sign++)
					{
						if (value->type != T_VOID)
							VALUE_conv(value, *sign);
					}
				}
			}

			if (UNLIKELY(nm < nparam))
				EXEC.nparvar = nparam - nm;
			else
				EXEC.nparvar = 0;

			for (; i < nparam; i++, value++)
				VARIANT_undo(value);

			//printf("EXEC_native: nparvar = %d\n", EXEC.nparvar);
		}
	}
	CATCH
	{
		RELEASE_MANY(SP, nparam);
		PROPAGATE();	
	}
	END_TRY

	EXEC_call_native_inline(desc->exec, object, desc->type, &SP[-nparam]);
	COPY_VALUE(&ret, &TEMP);

	/* Lib�ation des arguments */

	/*while (nparam > 0)
	{
		nparam--;
		POP();
	}*/
	
	RELEASE_MANY(SP, nparam);
	
	/* Si la description de la fonction se trouve sur la pile */

	if (use_stack)
	{
		SP--;
		free_later = SP->_function.object;
		SP->type = T_NULL;
	}

	/*
	#if DEBUG_STACK
	else
		printf("** SP != func SP = %p func = %p **\n>", SP, func);
	#endif
	*/

	if (LIKELY(!error))
	{
		if (desc->type == T_VOID)
		{
			if (!drop)
			{
				SP->type = T_VOID;
				SP->_void.ptype = T_NULL;
				SP++;
			}
		}
		else
		{
			BORROW(&ret);

			if (drop)
			{
				RELEASE(&ret);
			}
			else
			{
				//VALUE_conv(&ret, desc->type);
				//*SP = ret;
				COPY_VALUE(SP, &ret);
				SP++;
			}
		}
	}

	if (use_stack)
		OBJECT_UNREF(free_later, "EXEC_native (FUNCTION)");

	if (UNLIKELY(error))
		PROPAGATE();

	#if DEBUG_STACK
	printf("| << EXEC_native: %s (%p)\n", desc->name, &desc);
	#endif
}


bool EXEC_object(VALUE *val, CLASS **pclass, OBJECT **pobject)
{
	static const void *jump[] = {
		&&__ERROR, &&__ERROR, &&__ERROR, &&__ERROR, &&__ERROR, &&__ERROR, &&__ERROR, &&__ERROR, &&__ERROR,
		&&__ERROR, &&__ERROR, &&__ERROR, &&__VARIANT, &&__FUNCTION, &&__CLASS, &&__NULL,
		&&__OBJECT
		};

	CLASS *class;
	OBJECT *object;
	bool defined;

__RETRY:

	if (LIKELY(TYPE_is_pure_object(val->type)))
		goto __PURE_OBJECT;
	else
		goto *jump[val->type];

__FUNCTION:

	if (LIKELY(val->_function.kind == FUNCTION_UNKNOWN))
	{
		EXEC.property = TRUE;
		EXEC.unknown = CP->load->unknown[val->_function.index];

		EXEC_special(SPEC_UNKNOWN, val->_function.class, val->_function.object, 0, FALSE);

		object = val->_function.object;
		OBJECT_UNREF(object, "EXEC_object (FUNCTION)");

		SP--;
		//*val = *SP;
		COPY_VALUE(val, SP);
		goto __RETRY;
	}
	else
		goto __ERROR;

__CLASS:

	class = val->_class.class;
	object = NULL;
	defined = TRUE;

	if (val == EXEC_super)
	{
		EXEC_super = val->_class.super;
		//*class = (*class)->parent;
		if (UNLIKELY(class == NULL))
			THROW(E_PARENT);
	}
	
	CLASS_load(class);
	goto __RETURN;

__OBJECT:

	object = val->_object.object;
	if (!object)
		goto __NULL;
	class = object->class;
	defined = FALSE;

	goto __CHECK;

__PURE_OBJECT:

	object = val->_object.object;
	class = val->_object.class;
	defined = TRUE;

	if (!object)
	{
		/* A null object and a virtual class means that we want to pass a static class */
		if (!class->is_virtual)
			goto __NULL;
		CLASS_load(class);
		goto __RETURN;
	}
	
	if (UNLIKELY(val == EXEC_super))
	{
		EXEC_super = val->_object.super;
		//*class = (*class)->parent;
		if (UNLIKELY(class == NULL))
			THROW(E_PARENT);
	}
	else if (!class->is_virtual)
		class = object->class;

	goto __CHECK;

__VARIANT:

	if (val->_variant.vtype == T_OBJECT)
	{
		object = val->_variant.value._object;
		if (!object)
			goto __NULL;
		class = object->class;
		defined = FALSE;
		goto __CHECK;
	}
	else if (TYPE_is_object(val->_variant.vtype))
	{
		object = val->_variant.value._object;
		if (!object)
			goto __NULL;
		class = (CLASS *)val->_variant.vtype;
		if (!class->is_virtual)
			class = object->class; /* Virtual dispatching */
		defined = FALSE;
		goto __CHECK;
	}
	else
		goto __ERROR;

__ERROR:

	THROW(E_NOBJECT);

__NULL:

	THROW(E_NULL);

__CHECK:

	//CLASS_load(class); If we have an object, the class is necessarily loaded.

	if (UNLIKELY(class->must_check && (*(class->check))(object)))
		THROW(E_IOBJECT);
		
__RETURN:

	*pclass = class;
	*pobject = object;
	
	return defined;
}


void EXEC_public_desc(CLASS *class, void *object, CLASS_DESC_METHOD *desc, int nparam)
{
  EXEC.object = object;
  EXEC.nparam = nparam; /*desc->npmin;*/
  EXEC.drop = FALSE;

  if (FUNCTION_is_native(desc))
  {
		EXEC.class = class; // EXEC_native() does not need the real class, except the GB.GetClass(NULL) API used by Form.Main.
    EXEC.native = TRUE;
    EXEC.use_stack = FALSE;
    EXEC.desc = desc;
    EXEC_native();
    SP--;
    *RP = *SP;
    SP->type = T_VOID;
  }
  else
  {
		EXEC.class = desc->class; // EXEC_function_real() needs the effective class, because the method can be an inherited one!
    EXEC.native = FALSE;
    EXEC.index = (int)(intptr_t)desc->exec;
    EXEC_function_keep();
  }
}

void EXEC_public(CLASS *class, void *object, const char *name, int nparam)
{
	CLASS_DESC *desc;

	desc = CLASS_get_symbol_desc_kind(class, name, (object != NULL) ? CD_METHOD : CD_STATIC_METHOD, 0);

	if (UNLIKELY(desc == NULL))
		return;
	
	EXEC_public_desc(class, object, &desc->method, nparam);
	EXEC_release_return_value();
	
	#if 0
	EXEC.class = desc->method.class;
	EXEC.object = object;
	EXEC.nparam = nparam;
	EXEC.drop = TRUE;

	if (FUNCTION_is_native(&desc->method))
	{
		EXEC.desc = &desc->method;
		EXEC.use_stack = FALSE;
		EXEC_native();
	}
	else
	{
		EXEC.index = (int)(intptr_t)desc->method.exec;
		//EXEC.func = &class->load->func[(long)desc->method.exec]
		EXEC_function();
	}
	#endif
}



bool EXEC_special(int special, CLASS *class, void *object, int nparam, bool drop)
{
	CLASS_DESC *desc;
	short index = class->special[special];

	if (UNLIKELY(index == NO_SYMBOL))
		return TRUE;

	desc = CLASS_get_desc(class, index);

	if (CLASS_DESC_get_type(desc) == CD_STATIC_METHOD)
	{
		if (object != NULL)
			return TRUE;
	}
	else
	{
		if (object == NULL)
		{
			if (class->auto_create)
				object = EXEC_auto_create(class, FALSE);

			if (object == NULL)
				THROW(E_NOBJECT);
		}
	}

	EXEC.class = desc->method.class;
	EXEC.object = object;
	EXEC.nparam = nparam;
	EXEC.drop = drop;

	/*printf("<< EXEC_spec: SP = %d\n", SP - (VALUE *)STACK_base);
	save_SP = SP;*/

	if (FUNCTION_is_native(&desc->method))
	{
		EXEC.desc = &desc->method;
		EXEC.use_stack = FALSE;
		EXEC.native = TRUE;
		EXEC_native();
	}
	else
	{
		//EXEC.func = &class->load->func[(long)desc->method.exec]
		EXEC.index = (int)(intptr_t)desc->method.exec;
		EXEC.native = FALSE;
		if (drop)
			EXEC_function();
		else
		{
			EXEC_function_keep();
			//*SP++ = *RP;
			COPY_VALUE(SP, RP);
			SP++;
			RP->type = T_VOID;
		}
	}

	/*printf(">> EXEC_spec: SP = %d\n", SP - (VALUE *)STACK_base);
	if (SP != save_SP)
		printf("**** SP should be %d\n", save_SP - (VALUE *)STACK_base);*/

	return FALSE;
}


/* static void dump(int np) */
/* { */
/*   int i; */
/*  */
/*   for (i = 1; i <= np; i++) */
/*     printf("SP[%d] = %d  ", -i, SP[-i].type); */
/*  */
/*   printf("\n"); */
/* } */
/*  */

/*
	The highest parent method is called first, but get only the parameters
	not consumed by the child methods.
*/

#if 0
void EXEC_special_inheritance(int special, CLASS *class, OBJECT *object, int nparam, bool drop)
{
	CLASS *her[MAX_INHERITANCE];
	int npher[MAX_INHERITANCE];
	int nher;
	int i, np;
	CLASS_DESC *desc;
	short index;

	if (!class->parent)
	{
		if (special == SPEC_NEW && !CLASS_is_native(class))
		{
			EXEC.class = class;
			EXEC.object = object;
			EXEC.index = FUNC_INIT_DYNAMIC;
			EXEC.native = FALSE;
			EXEC.nparam = 0;
			EXEC_function();
		}
		EXEC_special(special, class, object, nparam, drop);
		return;
	}

	nher = CLASS_get_inheritance(class, her);

	for(i = 0, np = 0; i < nher; i++)
	{
		class = her[i];
		npher[i] = np;

		index = class->special[special];
		if (index == NO_SYMBOL)
			continue;
		desc = CLASS_get_desc(class, index); //class->special[special];

		np += desc->method.npmax;
	}

	for(;;)
	{
		nher--;
		if (nher < 0)
			break;
		class = her[nher];

		if (special == SPEC_NEW)
		{
			if (!CLASS_is_native(class))
			{
				EXEC.class = class;
				EXEC.object = object;
				EXEC.index = FUNC_INIT_DYNAMIC;
				//EXEC.func = &class->load->func[FUNC_INIT_DYNAMIC];
				EXEC.native = FALSE;
				EXEC.nparam = 0;

				EXEC_function();
			}
		}

		index = class->special[special];
		if (index == NO_SYMBOL)
			continue;

		desc = CLASS_get_desc(class, index); // class->special[special];

		/*np = Min(nparam, desc->method.npmax);*/

		np = Max(0, nparam - npher[nher]);

		EXEC_special(special, class, object, np, drop);
		nparam -= np;
	}
}
#endif

void EXEC_special_inheritance(int special, CLASS *class, OBJECT *object, int nparam, bool drop)
{
	CLASS *her[MAX_INHERITANCE];
	int nher;
	int i, np, npopt, nparam_opt;
	CLASS_DESC *desc;
	short index;
	int arg, opt;
	VALUE *base;

	if (!class->parent)
	{
		if (special == SPEC_NEW && class->init_dynamic)
		{
			EXEC.class = class;
			EXEC.object = object;
			EXEC.index = FUNC_INIT_DYNAMIC;
			EXEC.native = FALSE;
			EXEC.nparam = 0;
			EXEC_function();
		}
		EXEC_special(special, class, object, nparam, drop);
		return;
	}

	nher = CLASS_get_inheritance(class, her);

	for(i = 0, np = 0; i < nher; i++)
	{
		class = her[i];
	
		index = class->special[special];
		if (index == NO_SYMBOL)
			continue;
		desc = CLASS_get_desc(class, index); //class->special[special];
		np += desc->method.npmin;
	}

	if (UNLIKELY(np > nparam))
		THROW(E_NEPARAM);

	arg = - nparam;
	opt = arg + np;
	nparam_opt = nparam - np;
	nparam = np;
	
	// nparam is now the number of mandatory arguments
	// naram_opt is the number of optional arguments
	
	for(;;)
	{
		nher--;
		if (nher < 0)
			break;
		class = her[nher];

		if (special == SPEC_NEW)
		{
			if (class->init_dynamic)
			{
				EXEC.class = class;
				EXEC.object = object;
				EXEC.index = FUNC_INIT_DYNAMIC;
				//EXEC.func = &class->load->func[FUNC_INIT_DYNAMIC];
				EXEC.native = FALSE;
				EXEC.nparam = 0;

				EXEC_function();
			}
		}

		index = class->special[special];
		if (index == NO_SYMBOL)
			continue;

		desc = CLASS_get_desc(class, index); // class->special[special];

		if (nher)
		{
			np = desc->method.npmin;
			if (np > nparam) np = nparam;
			nparam -= np;
			
			npopt = desc->method.npmax - desc->method.npmin;
			if (npopt > nparam_opt) npopt = nparam_opt;
			nparam_opt -= npopt;
		}
		else
		{
			np = nparam;
			npopt = nparam_opt;
		}

		if (np > 0 || npopt > 0)
		{
			STACK_check(np + npopt);
			
			base = SP;
			
			for (i = 0; i < np; i++)
			{
				*SP++ = base[arg];
				base[arg].type = T_NULL;
				arg++;
			}

			for (i = 0; i < npopt; i++)
			{
				*SP++ = base[opt];
				base[opt].type = T_NULL;
				opt++;
			}
		}
		
		EXEC_special(special, class, object, np + npopt, drop);
	}
	
	SP -= nparam + nparam_opt;
}

void *EXEC_create_object(CLASS *class, int np, char *event)
{
	void *object;

	CLASS_load(class);

	if (UNLIKELY(class->no_create))
		THROW(E_CSTATIC, class->name);

	OBJECT_new(&object, class, event, ((OP == NULL) ? (OBJECT *)CP : (OBJECT *)OP));

	TRY
	{
		OBJECT_lock(object, TRUE);
		EXEC_special_inheritance(SPEC_NEW, class, object, np, TRUE);
		OBJECT_lock(object, FALSE);

//     SP--; /* class */
// 
//     SP->_object.class = class;
//     SP->_object.object = object;
//     SP++;
	}
	CATCH
	{
		// _free() methods should not be called, but we must
		OBJECT_UNREF(object, "EXEC_new");
		PROPAGATE();
//     SP--; /* class */
//     SP->type = T_NULL;
//     SP++;
//     PROPAGATE();
	}
	END_TRY

	return object;
}


void EXEC_new(void)
{
	CLASS *class;
	int np;
	bool event;
	void *object;
	char *name = NULL;
	char *cname = NULL;

	np = *PC & 0xFF;
	event = np & CODE_NEW_EVENT;
	np &= 0x3F;

	/* Instanciation */

	SP -= np;

	if (SP->type == T_CLASS)
	{
		class = SP->_class.class;
		if (UNLIKELY(class->override != NULL))
			class = class->override;
	}
	else if (TYPE_is_string(SP->type))
	{
		cname = STRING_copy_from_value_temp(SP);
		class = CLASS_find(cname);
		RELEASE_STRING(SP);
		SP->type = T_NULL;
	}
	else
		THROW(E_TYPE, "String", TYPE_get_name(SP->type));

	SP += np;

	//printf("**** NEW %s\n", class->name);
	CLASS_load(class);

	if (UNLIKELY(class->no_create))
		THROW(E_CSTATIC, class->name);

	if (event)
	{
		SP--;

		if (!TYPE_is_string(SP->type))
			THROW(E_TYPE, "String", TYPE_get_name(SP->type));

		name = STRING_copy_from_value_temp(SP);
		//printf("**** name %s\n", class->name);

		STRING_ref(name);
		SP++;
		OBJECT_new(&object, class, name, ((OP == NULL) ? (OBJECT *)CP : (OBJECT *)OP));
		SP--;
		STRING_unref(&name);

		RELEASE_STRING(SP);

		np -= 2;
	}
	else
	{
		OBJECT_new(&object, class, name, ((OP == NULL) ? (OBJECT *)CP : (OBJECT *)OP));
		np--;
	}

	/*OBJECT_REF(object, "EXEC_new");*/

	/* On retourne l'objet cr� */

	TRY
	{
		OBJECT_lock(object, TRUE);
		EXEC_special_inheritance(SPEC_NEW, class, object, np, TRUE);
		OBJECT_lock(object, FALSE);

		SP--; /* class */

		SP->_object.class = class;
		SP->_object.object = object;
		SP++;
	}
	CATCH
	{
		// _free() methods should not be called, but we must
		OBJECT_UNREF(object, "EXEC_new");
		//(*class->free)(class, object);
		SP--; /* class */
		SP->type = T_NULL;
		SP++;
		PROPAGATE();
	}
	END_TRY

	/*  PUSH(); */

	/* L'objet a ��cr� avec un nombre de r��ence �al �1.
		On remet ce nombre �0 maintenant que l'objet est pr�.
		Mais on ne le d�ruit pas ! */

	/* OBJECT_UNREF(&object, "EXEC_new"); */
}


void EXEC_quit(void)
{
	GAMBAS_DoNotRaiseEvent = TRUE;

	HOOK(quit)();

	THROW(E_ABORT);
}

void *EXEC_auto_create(CLASS *class, bool ref)
{
	void *object;

	object = CLASS_auto_create(class, 0); /* object is checked by CLASS_auto_create */
	if (ref)
		OBJECT_REF(object, "EXEC_auto_create");
	return object;
}

void EXEC_dup(int n)
{
	VALUE *src;
	
	STACK_check(n);
	
	src = SP - n;
	while (n > 0)
	{
		BORROW(src);
		*SP++ = *src++;
		n--;
	}
}
