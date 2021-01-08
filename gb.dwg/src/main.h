/***************************************************************************

  main.h

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

#ifndef __MAIN_H
#define __MAIN_H

#include "config.h"
#include "gb_common.h"
#include "gambas.h"

#define USE_WRITE
#include <dwg.h>
#include <dwg_api.h>

#ifndef _MAIN_C
extern GB_INTERFACE GB;
// Collections
extern GB_CLASS CLASS_DwgDocument;
extern GB_CLASS CLASS_DxfDocument;
extern GB_CLASS CLASS_SummaryInfo;
extern GB_CLASS CLASS_Header;
extern GB_CLASS CLASS_ModelSpace;
extern GB_CLASS CLASS_PaperSpace;
extern GB_CLASS CLASS_Blocks;
extern GB_CLASS CLASS_DimStyles;
extern GB_CLASS CLASS_Layers;
extern GB_CLASS CLASS_Linetypes;
extern GB_CLASS CLASS_RegisteredApplications;
extern GB_CLASS CLASS_TextStyles;
extern GB_CLASS CLASS_UCSs;
extern GB_CLASS CLASS_Viewports;
extern GB_CLASS CLASS_Views;
extern GB_CLASS CLASS_Dictionaries;
//extern GB_CLASS CLASS_PlotConfigurations;
extern GB_CLASS CLASS_Groups;
extern GB_CLASS CLASS_Colors;
extern GB_CLASS CLASS_Layouts;
extern GB_CLASS CLASS_MlineStyles;
extern GB_CLASS CLASS_MLeaderStyles;
extern GB_CLASS CLASS_Materials;
//extern GB_CLASS CLASS_PlotStyles;
extern GB_CLASS CLASS_DetailViewStyles;
extern GB_CLASS CLASS_SectionViewStyles;
extern GB_CLASS CLASS_VisualStyles;
extern GB_CLASS CLASS_Scales;
extern GB_CLASS CLASS_TableStyles;
extern GB_CLASS CLASS_WipeoutVars;
extern GB_CLASS CLASS_AssocNetworks;
extern GB_CLASS CLASS_PersSubentManagers;
extern GB_CLASS CLASS_AssocPersSubentManagers;

#define DWG_OBJECT(token) extern GB_CLASS CLASS_##token;
#define DWG_ENTITY(token) DWG_OBJECT(token)
#include "objects.inc"
#undef DWG_OBJECT
#undef DWG_ENTITY

#endif

#endif /* __MAIN_H */
