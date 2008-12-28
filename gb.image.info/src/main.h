/***************************************************************************

  main.h

  gb.image.info component

  (c) 2008 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __MAIN_H
#define __MAIN_H

#include "gambas.h"
#include "gb_common.h"

#ifndef __MAIN_C
extern GB_INTERFACE GB;
#endif

typedef
	struct {
		char *addr;
		int len;
		int pos;
		}
	IMAGE_STREAM;

int stream_seek(IMAGE_STREAM *stream, int pos, int whence);
int stream_read(IMAGE_STREAM *stream, void *addr, int len);
int stream_getc(IMAGE_STREAM *stream);

#endif /* __MAIN_H */
