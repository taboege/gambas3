/***************************************************************************

  SDLgfx.cpp

  (c) 2006 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#include "SDLgfx.h"
#include "SDLerror.h"
#include "SDLcore.h"
#include "SDLapp.h"
#include "SDLwindow.h"
#include "SDLsurface.h"
#include "SDLtexture.h"

#include <iostream>
#include <math.h>

// for ellipses
#define PI 3.14159265359

// debug
// #define DEBUG_GFX

// fill patterns
static GLubyte VertPattern[] = {0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 
				0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,	
				0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,	
				0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,	
				0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,	
				0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,	
				0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,	
				0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,	
				0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,	
				0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,	
				0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22};

static GLubyte HoriPattern[] = {0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,	
				0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,	
				0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 
				0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,	
				0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,	
				0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 
				0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,	
				0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00};

static GLubyte CrosPattern[] = {0xFF, 0xFF, 0xFF, 0xFF, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 
				0x22, 0x22, 0x22, 0x22, 0xFF, 0xFF, 0xFF, 0xFF, 0x22, 0x22, 0x22, 0x22,	
				0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0xFF, 0xFF, 0xFF, 0xFF,	
				0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,	
				0xFF, 0xFF, 0xFF, 0xFF, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,	
				0x22, 0x22, 0x22, 0x22, 0xFF, 0xFF, 0xFF, 0xFF, 0x22, 0x22, 0x22, 0x22,	
				0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0xFF, 0xFF, 0xFF, 0xFF,	
				0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,	
				0xFF, 0xFF, 0xFF, 0xFF, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,	
 				0x22, 0x22, 0x22, 0x22, 0xFF, 0xFF, 0xFF, 0xFF, 0x22, 0x22, 0x22, 0x22,	
				0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22};

static GLubyte BdiaPattern[] = {0x80, 0x80, 0x80, 0x80, 0x40, 0x40, 0x40, 0x40, 0x20, 0x20, 0x20, 0x20, 
				0x10, 0x10, 0x10, 0x10, 0x08, 0x08, 0x08, 0x08, 0x04, 0x04, 0x04, 0x04, 
				0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x80, 0x80, 0x80, 0x80, 
				0x40, 0x40, 0x40, 0x40, 0x20, 0x20, 0x20, 0x20, 0x10, 0x10, 0x10, 0x10, 
				0x08, 0x08, 0x08, 0x08, 0x04, 0x04, 0x04, 0x04, 0x02, 0x02, 0x02, 0x02, 
				0x01, 0x01, 0x01, 0x01, 0x80, 0x80, 0x80, 0x80, 0x40, 0x40, 0x40, 0x40, 
				0x20, 0x20, 0x20, 0x20, 0x10, 0x10, 0x10, 0x10, 0x08, 0x08, 0x08, 0x08, 
				0x04, 0x04, 0x04, 0x04, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 
				0x80, 0x80, 0x80, 0x80, 0x40, 0x40, 0x40, 0x40, 0x20, 0x20, 0x20, 0x20, 
				0x10, 0x10, 0x10, 0x10, 0x08, 0x08, 0x08, 0x08, 0x04, 0x04, 0x04, 0x04, 
				0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01};

static GLubyte DiaPattern[] =  {0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04, 0x04, 
				0x08, 0x08, 0x08, 0x08, 0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20, 
				0x40, 0x40, 0x40, 0x40, 0x80, 0x80, 0x80, 0x80, 0x01, 0x01, 0x01, 0x01, 
				0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04, 0x04, 0x08, 0x08, 0x08, 0x08, 
				0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20, 0x40, 0x40, 0x40, 0x40, 
				0x80, 0x80, 0x80, 0x80, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 
				0x04, 0x04, 0x04, 0x04, 0x08, 0x08, 0x08, 0x08, 0x10, 0x10, 0x10, 0x10, 
				0x20, 0x20, 0x20, 0x20, 0x40, 0x40, 0x40, 0x40, 0x80, 0x80, 0x80, 0x80, 
				0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04, 0x04, 
				0x08, 0x08, 0x08, 0x08, 0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20, 
				0x40, 0x40, 0x40, 0x40, 0x80, 0x80, 0x80, 0x80};

static GLubyte DiaCPattern[] = {0x81, 0x81, 0x81, 0x81, 0x42, 0x42, 0x42, 0x42, 0x24, 0x24, 0x24, 0x24, 
				0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24, 0x24, 
				0x42, 0x42, 0x42, 0x42, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 
				0x42, 0x42, 0x42, 0x42, 0x24, 0x24, 0x24, 0x24, 0x18, 0x18, 0x18, 0x18, 
				0x18, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24, 0x24, 0x42, 0x42, 0x42, 0x42, 
				0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x42, 0x42, 0x42, 0x42, 
				0x24, 0x24, 0x24, 0x24, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
				0x24, 0x24, 0x24, 0x24, 0x42, 0x42, 0x42, 0x42, 0x81, 0x81, 0x81, 0x81, 
				0x81, 0x81, 0x81, 0x81, 0x42, 0x42, 0x42, 0x42, 0x24, 0x24, 0x24, 0x24, 
				0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24, 0x24, 
				0x42, 0x42, 0x42, 0x42, 0x81, 0x81, 0x81, 0x81};

static GLubyte Dns1Pattern[] = {0xDD, 0xDD, 0xDD, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
				0xFF, 0xFF, 0xFF, 0xFF, 0xDD, 0xDD, 0xDD, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xDD, 0xDD, 0xDD, 0xDD, 
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
				0xDD, 0xDD, 0xDD, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
				0xFF, 0xFF, 0xFF, 0xFF, 0xDD, 0xDD, 0xDD, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xDD, 0xDD, 0xDD, 0xDD, 
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
				0xDD, 0xDD, 0xDD, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
				0xFF, 0xFF, 0xFF, 0xFF, 0xDD, 0xDD, 0xDD, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static GLubyte Dns2Pattern[] = {0xDD, 0xDD, 0xDD, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0x77, 0x77, 0x77, 0x77, 
				0xFF, 0xFF, 0xFF, 0xFF, 0xDD, 0xDD, 0xDD, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 
				0x77, 0x77, 0x77, 0x77, 0xFF, 0xFF, 0xFF, 0xFF, 0xDD, 0xDD, 0xDD, 0xDD, 
				0xFF, 0xFF, 0xFF, 0xFF, 0x77, 0x77, 0x77, 0x77, 0xFF, 0xFF, 0xFF, 0xFF, 
				0xDD, 0xDD, 0xDD, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0x77, 0x77, 0x77, 0x77, 
				0xFF, 0xFF, 0xFF, 0xFF, 0xDD, 0xDD, 0xDD, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 
				0x77, 0x77, 0x77, 0x77, 0xFF, 0xFF, 0xFF, 0xFF, 0xDD, 0xDD, 0xDD, 0xDD, 
				0xFF, 0xFF, 0xFF, 0xFF, 0x77, 0x77, 0x77, 0x77, 0xFF, 0xFF, 0xFF, 0xFF, 
				0xDD, 0xDD, 0xDD, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0x77, 0x77, 0x77, 0x77, 
				0xFF, 0xFF, 0xFF, 0xFF, 0xDD, 0xDD, 0xDD, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 
				0x77, 0x77, 0x77, 0x77, 0xFF, 0xFF, 0xFF, 0xFF};

static GLubyte Dns3Pattern[] = {0xDD, 0xDD, 0xDD, 0xDD, 0xAA, 0xAA, 0xAA, 0xAA, 0x77, 0x77, 0x77, 0x77, 
				0xAA, 0xAA, 0xAA, 0xAA, 0xDD, 0xDD, 0xDD, 0xDD, 0xAA, 0xAA, 0xAA, 0xAA, 
				0x77, 0x77, 0x77, 0x77, 0xAA, 0xAA, 0xAA, 0xAA, 0xDD, 0xDD, 0xDD, 0xDD, 
				0xAA, 0xAA, 0xAA, 0xAA, 0x77, 0x77, 0x77, 0x77, 0xAA, 0xAA, 0xAA, 0xAA, 
				0xDD, 0xDD, 0xDD, 0xDD, 0xAA, 0xAA, 0xAA, 0xAA, 0x77, 0x77, 0x77, 0x77, 
				0xAA, 0xAA, 0xAA, 0xAA, 0xDD, 0xDD, 0xDD, 0xDD, 0xAA, 0xAA, 0xAA, 0xAA, 
				0x77, 0x77, 0x77, 0x77, 0xAA, 0xAA, 0xAA, 0xAA, 0xDD, 0xDD, 0xDD, 0xDD, 
				0xAA, 0xAA, 0xAA, 0xAA, 0x77, 0x77, 0x77, 0x77, 0xAA, 0xAA, 0xAA, 0xAA, 
				0xDD, 0xDD, 0xDD, 0xDD, 0xAA, 0xAA, 0xAA, 0xAA, 0x77, 0x77, 0x77, 0x77, 
				0xAA, 0xAA, 0xAA, 0xAA, 0xDD, 0xDD, 0xDD, 0xDD, 0xAA, 0xAA, 0xAA, 0xAA, 
				0x77, 0x77, 0x77, 0x77, 0xAA, 0xAA, 0xAA, 0xAA};

static GLubyte Dns4Pattern[] = {0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 
				0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 
				0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 
				0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 
				0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 
				0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 
				0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 
				0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 
				0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 
				0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 
				0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA};

static GLubyte Dns5Pattern[] = {0xAA, 0xAA, 0xAA, 0xAA, 0x44, 0x44, 0x44, 0x44, 0xAA, 0xAA, 0xAA, 0xAA, 
				0x11, 0x11, 0x11, 0x11, 0xAA, 0xAA, 0xAA, 0xAA, 0x44, 0x44, 0x44, 0x44, 
				0xAA, 0xAA, 0xAA, 0xAA, 0x11, 0x11, 0x11, 0x11, 0xAA, 0xAA, 0xAA, 0xAA, 
				0x44, 0x44, 0x44, 0x44, 0xAA, 0xAA, 0xAA, 0xAA, 0x11, 0x11, 0x11, 0x11, 
				0xAA, 0xAA, 0xAA, 0xAA, 0x44, 0x44, 0x44, 0x44, 0xAA, 0xAA, 0xAA, 0xAA, 
				0x11, 0x11, 0x11, 0x11, 0xAA, 0xAA, 0xAA, 0xAA, 0x44, 0x44, 0x44, 0x44, 
				0xAA, 0xAA, 0xAA, 0xAA, 0x11, 0x11, 0x11, 0x11, 0xAA, 0xAA, 0xAA, 0xAA, 
				0x44, 0x44, 0x44, 0x44, 0xAA, 0xAA, 0xAA, 0xAA, 0x11, 0x11, 0x11, 0x11, 
				0xAA, 0xAA, 0xAA, 0xAA, 0x44, 0x44, 0x44, 0x44, 0xAA, 0xAA, 0xAA, 0xAA, 
				0x11, 0x11, 0x11, 0x11, 0xAA, 0xAA, 0xAA, 0xAA, 0x44, 0x44, 0x44, 0x44, 
				0xAA, 0xAA, 0xAA, 0xAA, 0x11, 0x11, 0x11, 0x11};

static GLubyte Dns6Pattern[] = {0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00, 0x88, 0x88, 0x88, 0x88, 
				0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00, 
				0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 
				0x00, 0x00, 0x00, 0x00, 0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00, 
				0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00, 0x88, 0x88, 0x88, 0x88, 
				0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00, 
				0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 
				0x00, 0x00, 0x00, 0x00, 0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00, 
				0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00, 0x88, 0x88, 0x88, 0x88, 
				0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00, 
				0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x00, 0x00};

static GLubyte Dns7Pattern[] = {0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static void SetLinePattern(int value)
{
	GLushort pattern = 0xFFFF;

	if (value == SDL::SolidLine)
		return;
	if (value == SDL::DotLine)
		pattern = 0xCCCC;
	if (value == SDL::DashLine)
		pattern = 0xAAAA;
	if (value == SDL::DashDotLine)
		pattern = 0xE4E4;
	if (value == SDL::DashDotDotLine)
		pattern = 0xF98C;

	glEnable(GL_LINE_STIPPLE);
	glLineStipple(2, pattern);
}

static void SetFillPattern(int value)
{
	if (!value)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (value <= SDL::SolidFill)
		return;

	glEnable(GL_POLYGON_STIPPLE);

	if (value == SDL::VerticalFill)
		glPolygonStipple(VertPattern);

	if (value == SDL::HorizontalFill)
		glPolygonStipple(HoriPattern);

	if (value == SDL::CrossFill)
		glPolygonStipple(CrosPattern);

	if (value == SDL::BackDiagFill)
		glPolygonStipple(BdiaPattern);

	if (value == SDL::DiagFill)
		glPolygonStipple(DiaPattern);

	if (value == SDL::DiagCrossFill)
		glPolygonStipple(DiaCPattern);

	if (value == SDL::Dense1Fill)
		glPolygonStipple(Dns1Pattern);

	if (value == SDL::Dense2Fill)
		glPolygonStipple(Dns2Pattern);

	if (value == SDL::Dense3Fill)
		glPolygonStipple(Dns3Pattern);

	if (value == SDL::Dense4Fill)
		glPolygonStipple(Dns4Pattern);

	if (value == SDL::Dense5Fill)
		glPolygonStipple(Dns5Pattern);

	if (value == SDL::Dense6Fill)
		glPolygonStipple(Dns6Pattern);

	if (value == SDL::Dense7Fill)
		glPolygonStipple(Dns7Pattern);
}

SDLgfx::SDLgfx(SDLwindow *window)
{
	hTex = NULL;
	resetGfx();
}

SDLgfx::SDLgfx(SDLsurface *surface)
{
	if (!SDLcore::GetWindow())
	{
		SDLerror::RaiseError("Window need to be opened first !");
		return;
	}

	hTex = surface->GetTexture();
	resetGfx();
}

void SDLgfx::resetGfx(void)
{
	hLine = SDL::SolidLine;
	hLineWidth = 1;
	hFill = SDL::NoFill;
	rotx = roty = rotz = 0;
	scalex = scaley = 1.0f;
}

void SDLgfx::SetColor(Uint32 color)
{
	glColor4f((GLfloat((color >> 16) & 0xFF)/255), (GLfloat((color >> 8) & 0xFF)/255), (GLfloat(color  & 0xFF)/255),
                (GLfloat(~(color >> 24) & 0xFF)/255));
}

void SDLgfx::SetLineStyle(int style)
{
	if (style>SDL::DashDotDotLine)
		style = SDL::DashDotDotLine;

	hLine = style;
}

void SDLgfx::SetFillStyle(int style)
{
	if (style>SDL::Dense7Fill)
		style = SDL::Dense7Fill;

	hFill = style;
}

void SDLgfx::Clear(void)
{
	SetContext();

 	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void SDLgfx::DrawPixel(int x, int y)
{
	SetContext();

	glBegin(GL_POINTS);
	glVertex2i(x, y);
	glEnd();
}

void SDLgfx::DrawLine(int x1, int y1, int x2, int y2)
{
	if (!hLine) // SDLgfx::NoLine
		return;

	SetContext();

	SetLinePattern(hLine);
	glLineWidth(GLfloat(hLineWidth));

	glBegin(GL_LINES);
	glVertex2i(x1, y1);
	glVertex2i(x2, y2);
	glEnd();
}

void SDLgfx::DrawRect(int x, int y, int w, int h)
{
	if (!hFill && !hLine)
		return;

	SetContext();

	SetFillPattern(hFill);

	glBegin(GL_QUADS);
	glVertex2i(x, y);
	glVertex2i(x+w, y);
	glVertex2i(x+w, y+h);
	glVertex2i(x, y+h);
	glEnd();

	if (hFill>SDL::SolidFill)
	{
		SetFillPattern(SDL::NoFill);
		SetLinePattern(hLine);
		glLineWidth(GLfloat(hLineWidth));

		glBegin(GL_QUADS);
		glVertex2i(x, y);
		glVertex2i(x+w, y);
		glVertex2i(x+w, y+h);
		glVertex2i(x, y+h);
		glEnd();
	}
}

void SDLgfx::DrawEllipse(int x, int y, int w, int h)
{
	if (!hFill && !hLine)
		return;

	SetContext();

	double angle;
	double step = 2 * PI / 360;

	glTranslatef(x, y, 0.0f);
	SetFillPattern(hFill);

	glBegin(GL_POLYGON);
	for (angle=0; angle < 2 * PI; angle += step)
		glVertex2d(w * cos(angle), h * sin(angle));
	glEnd();

	if (hFill>SDL::SolidFill)
	{
		SetFillPattern(SDL::NoFill);
		SetLinePattern(hLine);
		glLineWidth(GLfloat(hLineWidth));
		glBegin(GL_POLYGON);
		for (angle=0; angle < 2 * PI; angle += step)
			glVertex2d(w * cos(angle), h * sin(angle));
		glEnd();
	}

	glLoadIdentity();
}

void SDLgfx::Blit(SDLsurface *surface, int x, int y, int srcX, int srcY,
		 int srcWidth, int srcHeight, int width, int height)
{
	if ((srcX > surface->GetWidth()) || (srcY > surface->GetHeight()))
		return;

	if (!surface->GetWidth() || !surface->GetHeight())
		return;
	
	SDL_Surface *destsurf = GetDestSurface();

	if ((x > destsurf->w) || (y > destsurf->h))
		return;

	SetContext();

	texinfo info;

	glPushAttrib(GL_TEXTURE_BIT);
	SDLtexture *texture = surface->GetTexture();
	texture->GetAsTexture(&info);

	GLfloat myWidth = 0, myHeight = 0;

	if ((srcHeight<0) || ((srcY + srcHeight) > surface->GetHeight()))
		myHeight = surface->GetHeight() - srcY;
	else
		myHeight = srcHeight;

	if ((srcWidth<0) || ((srcX + srcWidth) > surface->GetWidth()))
		myWidth = surface->GetWidth() - srcX;
	else
		myWidth = srcWidth;

	GLdouble myTexX, myTexY, myTexHeight, myTexWidth;

	myTexX = ((srcX * info.Width) / surface->GetWidth());
	myTexY = ((srcY * info.Height) / surface->GetHeight());
	myTexWidth = (((srcX + myWidth)* info.Width) / surface->GetWidth());
	myTexHeight = (((srcY + myHeight)* info.Height) / surface->GetHeight());

	if (width != -1)
		myWidth = width;

	if (height != -1)
		myHeight = height;

	myWidth = myWidth / 2;
	myHeight = myHeight / 2;
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, info.Index);

	glTranslatef(myWidth + x, myHeight + y, 0.0f);
	glRotatef(rotz, 0, 0, 1);
	glScalef(scalex, scaley, 0.0f);
	
	glBegin(GL_QUADS);
	glTexCoord2d(myTexX, myTexY);
	glVertex2f(-myWidth, -myHeight);

	glTexCoord2d(myTexX, myTexHeight);
	glVertex2f(-myWidth, myHeight);

	glTexCoord2d(myTexWidth, myTexHeight);
	glVertex2f(myWidth, myHeight);

	glTexCoord2d(myTexWidth, myTexY);
	glVertex2f(myWidth, -myHeight);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	glPopAttrib();
	glLoadIdentity();
}

void SDLgfx::SetContext()
{
	if (!hTex)
		SDLcore::GetWindow()->Select();
	else
		hTex->Select();
}

SDL_Surface *SDLgfx::GetDestSurface()
{
	if (!hTex)
		return SDLcore::GetWindow()->GetSdlSurface();
	else
		return hTex->GetSurface()->GetSdlSurface();
}

