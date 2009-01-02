/***************************************************************************

  String.h

  The String management routines

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __STRING_H
#define __STRING_H

#include "gbx_value.h"
#include "gbx_debug.h"
#include "gb_common_string.h"

typedef
  struct
  {
    int ref;
    int len;
    char data[0];
  }
  PACKED
  STRING;

typedef
  void (*SUBST_FUNC)(int, char **, int *);

typedef
  void (*SUBST_ADD_FUNC)(int);

#define STRING_MAKE_TEMP 32

typedef
	struct
	{
		char *buffer;
		char *ptr;
		int inc;
		int len;
		int max;
		char temp[STRING_MAKE_TEMP];
		int ntemp;
	}
	STRING_MAKE;

#define SC_UNICODE ((char *)-1)

#ifndef __STRING_C
extern STRING_MAKE STRING_make_buffer;
#endif

void STRING_init(void);
void STRING_exit(void);

void STRING_new(char **ptr, const char *src, int len);
void STRING_free_real(char *ptr);
void STRING_free_later(char *ptr);
int STRING_get_free_index(void);

#define STRING_new_temp(_pptr, _src, _len) STRING_new(_pptr, _src, _len), STRING_free_later(*_pptr)

void STRING_extend(char **ptr, int new_len);
void STRING_extend_end(char **ptr);
void STRING_add(char **ptr, const char *src, int len);
void STRING_add_char(char **ptr, char c);

void STRING_copy_from_value_temp(char **ptr, VALUE *value);

void STRING_new_temp_value(VALUE *value, const char *src, int len);
void STRING_new_constant_value(VALUE *value, const char *src, int len);

void STRING_char_value(VALUE *value, uchar car);
void STRING_void_value(VALUE *value);

char *STRING_subst(const char *str, int len, SUBST_FUNC get_param);
char *STRING_subst_add(const char *str, int len, SUBST_ADD_FUNC add_param);
int STRING_conv(char **result, const char *str, int len, const char *src, const char *dst, bool throw);
char *STRING_conv_file_name(const char *name, int len);
char *STRING_conv_to_UTF8(const char *name, int len);

#define STRING_from_ptr(_ptr) ((STRING *)((_ptr) - offsetof(STRING, data)))
#define STRING_length(_ptr) ((_ptr) == NULL ? 0 : STRING_from_ptr(_ptr)->len)

#if DEBUG_STRING

void STRING_free(char **ptr);
void STRING_ref(char *ptr);
void STRING_unref(char **ptr);

#else

#define STRING_free(_p) \
({ \
  char **pptr = _p; \
  char *ptr = *pptr; \
  if (ptr) \
  { \
  	STRING_free_real(ptr); \
  	*pptr = NULL; \
  } \
})

#define STRING_ref(_p) \
({ \
  char *ptr = _p; \
  if (ptr) \
    STRING_from_ptr(ptr)->ref++; \
})

#define STRING_unref(_p) \
({ \
  char **pptr = _p; \
  char *ptr = *pptr; \
  STRING *str; \
  if (ptr) \
  { \
	  str = STRING_from_ptr(ptr); \
  	if ((--str->ref) <= 0) \
  	{ \
	  	STRING_free_real(ptr); \
    	*pptr = NULL; \
    } \
  } \
})

#endif

void STRING_unref_keep(char **ptr);

int STRING_search(const char *ps, int ls, const char *pp, int lp, int is, bool right, bool nocase);

void STRING_start_len(int len);
#define STRING_start() STRING_start_len(0)
char *STRING_end();
char *STRING_end_temp();
void STRING_make(const char *src, int len);
void STRING_make_dump();

#define STRING_make_char(_c) \
({ \
	if (STRING_make_buffer.ntemp == STRING_MAKE_TEMP) \
		STRING_make_dump(); \
	STRING_make_buffer.temp[STRING_make_buffer.ntemp++] = (_c); \
})

#endif
