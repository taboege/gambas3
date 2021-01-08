/***************************************************************************

  c_ents.h

  (C) 2020 Reini Urban <rurban@cpan.org>

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

#ifndef _C_ENTS_H
#define _C_ENTS_H

#include "c_dwg.h"

#define ENTITY_COLLECTION(token)                   \
  typedef struct {                                 \
    GB_BASE ob;                                    \
    Dwg_Data *dwg;                                 \
    Dwg_Object_BLOCK_HEADER *blkhdr;               \
    /* list of entities: */                        \
    unsigned iter;                                 \
  } C##token

ENTITY_COLLECTION (ModelSpace);
ENTITY_COLLECTION (PaperSpace);
ENTITY_COLLECTION (Blocks);

#endif
