/***************************************************************************

  main.c

  gb.dwg component

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

#define _MAIN_C

#include "main.h"
#include "c_dwg.h"

GB_INTERFACE GB EXPORT;

GB_DESC *GB_CLASSES[] EXPORT =
{
  DwgDocument_Desc,     /* The main DWG */
    DxfDocument_Desc,   /* an alias */
  SummaryInfo_Desc,
  Header_Desc,          /* i.e. Database */
  ModelSpace_Desc,
  PaperSpace_Desc,
  Blocks_Desc,
  DimStyles_Desc,
  Layers_Desc,
  Linetypes_Desc,
  RegisteredApplications_Desc,
  TextStyles_Desc,
  UCSs_Desc,
  Viewports_Desc,
  Views_Desc,

  Dictionaries_Desc,
  //PlotConfigurations_Desc,
  Groups_Desc,
  //Layouts_Desc,
  //MlineStyles_Desc,
  //MLeaderStyles_Desc,
  //Materials_Desc,
  //PlotStyles_Desc,
  //DetailViewStyles_Desc,
  //SectionViewStyles_Desc,
  //ViewStyles_Desc,
  //Scales_Desc,
  //TableStyles_Desc,
  //WipeoutVars_Desc,
  //AssocNetworks_Desc,
  //AssocPersSubentManagers_Desc,

  #define DWG_OBJECT(klass) klass##_Desc,
  #define DWG_ENTITY(klass) klass##_Desc,
  #include "objects.inc"
  #undef DWG_OBJECT
  #undef DWG_ENTITY
    
  NULL
};

// Collections
GB_CLASS CLASS_DwgDocument;
GB_CLASS CLASS_DxfDocument;
GB_CLASS CLASS_SummaryInfo;
GB_CLASS CLASS_Header;
GB_CLASS CLASS_ModelSpace;
GB_CLASS CLASS_PaperSpace;
GB_CLASS CLASS_Blocks;
GB_CLASS CLASS_DimStyles;
GB_CLASS CLASS_Layers;
GB_CLASS CLASS_Linetypes;
GB_CLASS CLASS_RegisteredApplications;
GB_CLASS CLASS_TextStyles;
GB_CLASS CLASS_UCSs;
GB_CLASS CLASS_Viewports;
GB_CLASS CLASS_Views;
GB_CLASS CLASS_Dictionaries;
GB_CLASS CLASS_Groups;
//GB_CLASS CLASS_PlotConfigurations;

// LibreDWG internal classes. see src/objects.inc
#define DWG_OBJECT(klass) GB_CLASS CLASS_##klass;
#define DWG_ENTITY(token) DWG_OBJECT(token)
#include "objects.inc"
#undef DWG_OBJECT
#undef DWG_ENTITY

//static void error_handler(const char *reason, const char *file, int line, int errno)
//{
//  GB.Error("&1: &2", dwg_errstrings(errno), reason);
//}

int EXPORT GB_INIT(void)
{
  // VBA-like public classes as collections
  CLASS_DwgDocument = GB.FindClass("DwgDocument");
  CLASS_DxfDocument = GB.FindClass("DxfDocument");
  CLASS_SummaryInfo = GB.FindClass("SummaryInfo");
  CLASS_Header = GB.FindClass("Header"); // ie Database
  CLASS_ModelSpace = GB.FindClass("ModelSpace");
  CLASS_PaperSpace = GB.FindClass("PaperSpace");
  CLASS_Blocks = GB.FindClass("Blocks");
  CLASS_DimStyles = GB.FindClass("DimStyles");
  CLASS_Layers = GB.FindClass("Layers");
  CLASS_Linetypes = GB.FindClass("Linetypes");
  CLASS_RegisteredApplications = GB.FindClass("RegisteredApplications");
  CLASS_TextStyles = GB.FindClass("TextStyles");
  CLASS_UCSs = GB.FindClass("UCSs");
  CLASS_Viewports = GB.FindClass("Viewports");
  CLASS_Views = GB.FindClass("Views");
  CLASS_Dictionaries = GB.FindClass("Dictionaries");
  //CLASS_PlotConfigurations = GB.FindClass("PlotConfigurations");
  CLASS_Groups = GB.FindClass("Groups");

  #define DWG_OBJECT(klass) CLASS_##klass = GB.FindClass(#klass);
  #define DWG_ENTITY(klass) CLASS_##klass = GB.FindClass(#klass);
  #include "objects.inc"
  #undef DWG_OBJECT
  #undef DWG_ENTITY

  //set_error_handler (error_handler);
}

void EXPORT GB_EXIT()
{
}
