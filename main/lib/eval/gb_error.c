/***************************************************************************

  error.c

  Errors management routines

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

#define __GB_ERROR_C

#include "main.h"
#include "gb_common.h"
#include <stdarg.h>
#include "gb_error.h"


/*PUBLIC ERROR_INFO ERROR_info;*/

#define NBR_ERR_MSG 3

static ERROR_CONTEXT *_current = NULL;

/*
static void *_must_free[MAX_MUST_FREE] = { 0 };
static int _must_free_index = 0;
*/

PUBLIC void ERROR_clear(void)
{
  errno = 0;
}


PUBLIC void ERROR_enter(ERROR_CONTEXT *err)
{
  CLEAR(err);
  err->prev = _current;
  _current = err;
}


PUBLIC void ERROR_leave(ERROR_CONTEXT *err)
{
  if (err->prev != ERROR_LEAVE_DONE)
  {
    /*ERROR_panic("ERROR_leave already done");*/

    _current = err->prev;
    err->prev = ERROR_LEAVE_DONE;
  }
}


PUBLIC char *ERROR_get(void)
{
  /*
  if (code > 0 && code < 256)
    return strerror(code);
  else
    return ERROR_Message[code - 256];
  */
  return strerror(errno);
}

#if 0
PUBLIC void ERROR_define(char *pattern, char *arg[])
{
  int n;
  uchar c;
  boolean subst;

  void _add_char(uchar c)
  {
    if (n >= MAX_ERROR_MSG)
      return;

    ERROR_info.msg[n++] = c;
  }

  void _add_string(char *s)
  {
    while (*s)
    {
      _add_char(*s);
      s++;
    }
  }

  if ((int)pattern > 0 && (int)pattern < NBR_ERR_MSG)
  {
    ERROR_info.code = (int)pattern;
    pattern = _message[(int)pattern];
  }
  else
    ERROR_info.code = -1;

  n = 0;

  if (arg)
  {
    subst = FALSE;

    for (;;)
    {
      c = *pattern++;
      if (c == 0)
        break;

      if (subst)
      {
        if (c >= '1' && c <= '4')
          _add_string(arg[c - '1']);
        else
        {
          _add_char('&');
          _add_char(c);
        }
        subst = FALSE;
      }
      else
      {
        if (c == '&')
          subst = TRUE;
        else
          _add_char(c);
      }
    }
  }
  else
  {
    _add_string(pattern);
  }

  _add_char(0);
}
#endif

PUBLIC void PROPAGATE()
{
  ERROR_CONTEXT *err;

  /*
  if (_must_free_index)
  {
    for (i = 0; i < _must_free_index; i++)
      OBJECT_UNREF(&_must_free[i], "PROPAGATE");

    _must_free_index = 0;
  }
  */

  if (_current == NULL)
    ERROR_panic("Cannot propagate error. No error handler.");

  err = _current;
  ERROR_leave(_current);
  longjmp(err->env, 1);
}

PUBLIC void THROW(const char *msg)
{
  GB.Error("&1", (char *)msg);
  PROPAGATE();
}

PUBLIC void THROW2(const char *pattern, const char *msg)
{
  GB.Error((char *)pattern, (char *)msg);
  PROPAGATE();
}

PUBLIC void ERROR_panic(char *error, ...)
{
  va_list args;

  va_start(args, error);

  fflush(NULL);

  fprintf(stderr, "\n"
                  "** INTERNAL ERROR **\n"
                  );
  vfprintf(stderr, error, args);
  fprintf(stderr, "\n");
  /*
  if (ERROR_info.code)
  {
    fprintf(stderr, "\n");
    ERROR_print();
    fprintf(stderr, "\n");
  }
  */
  fprintf(stderr, "** Program aborting... Sorry... :-(\n\n");
  abort();
}

/*
PUBLIC void ERROR_print_at(FILE *where)
{
  fprintf(where, "%s\n", ERROR_info.msg);
}

PUBLIC void ERROR_print(void)
{
  ERROR_print_at(stderr);
}
*/

/*
PUBLIC void ERROR_must_free(void *object)
{
  if (object)
  {
    if (_must_free_index >= MAX_MUST_FREE)
      ERROR_panic("ERROR_must_free: Too many calls");

    _must_free[_must_free_index++] = object;
  }
  else
  {
    if (_must_free_index <= 0)
      ERROR_panic("ERROR_must_free: Nothing to free");

    _must_free_index--;
    OBJECT_UNREF(&_must_free[_must_free_index], "ERROR_must_free");
  }
}
*/
