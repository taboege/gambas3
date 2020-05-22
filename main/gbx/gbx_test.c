/***************************************************************************

  gbx_test.c

  (c) Benoît Minisini <g4mba5@gmail.com>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __GBX_TEST_C

#include "gb_common.h"
#include "gbx_class.h"
#include "gbx_api.h"
#include "gbx_project.h"
#include "gbx_exec.h"
#include "gbx_test.h"

static void error_test_run(CARRAY *tests)
{
	OBJECT_UNREF(tests);
}

void TEST_run(const char *test_list)
{
	CLASS_DESC_METHOD *startup;
	CARRAY *tests;
	int i;
	char *test;
	char *p;
	char *name;
	CLASS *class;
	

	if (!test_list || !*test_list)
	{
		startup = (CLASS_DESC_METHOD *)CLASS_get_symbol_desc_kind(PROJECT_class, "_list", CD_STATIC_METHOD, 0, T_ANY);
		if (!startup)
			ERROR_panic("Test._List() method unavailable");
		EXEC_public_desc(PROJECT_class, NULL, startup, 0);
		return;
	}
	
	if (test_list[0] == '*' && !test_list[1])
	{
		startup = (CLASS_DESC_METHOD *)CLASS_get_symbol_desc_kind(PROJECT_class, "_runall", CD_STATIC_METHOD, 0, T_ANY);
		if (!startup)
			ERROR_panic("Test._RunAll() method unavailable");
		EXEC_public_desc(PROJECT_class, NULL, startup, 0);
		return;
	}

	startup = (CLASS_DESC_METHOD *)CLASS_get_symbol_desc_kind(PROJECT_class, "_run", CD_STATIC_METHOD, 0, T_ANY);
	if (!startup)
		ERROR_panic("Test._Run() method unavailable");
	
	tests = STRING_split(test_list, strlen(test_list), ",", 1, NULL, 0, TRUE, FALSE);

	ON_ERROR_1(error_test_run, tests)
	{
		for (i = 0; i < tests->count; i++)
		{
			test = *(char **)CARRAY_get_data_unsafe(tests, i);
			p = index(test, '.');
			if (p)
				name = STRING_new_temp(test, p - test);
			else
				name = test;

			class = CLASS_find(name);
			CLASS_load(class);
			
			GB_Push(2, T_OBJECT, class, T_STRING, test, STRING_length(test));
			EXEC_public_desc(PROJECT_class, NULL, startup, 2);
		}
	}
	END_ERROR
	
	OBJECT_UNREF(tests);
}

