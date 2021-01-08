/***************************************************************************

  c_dwg.h

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

#ifndef _C_DWG_H
#define _C_DWG_H

#include "gambas.h"
#include "main.h"

#include <stdint.h>

#ifndef _C_DWG_C

#define CLASSDESC(klass) extern GB_DESC klass##_Desc[]
#define ENTITY_COLLECTION(token) CLASSDESC(token)

CLASSDESC (DwgDocument);
CLASSDESC (DxfDocument);
CLASSDESC (SummaryInfo);
CLASSDESC (Header); // i.e. Database
ENTITY_COLLECTION (ModelSpace);
ENTITY_COLLECTION (PaperSpace);
ENTITY_COLLECTION (Blocks);
#undef ENTITY_COLLECTION

#define TABLE_COLLECTION(token,key) CLASSDESC(token)
TABLE_COLLECTION (DimStyles, DIMSTYLE);
TABLE_COLLECTION (Layers, LAYER);
TABLE_COLLECTION (Linetypes, LTYPE);
TABLE_COLLECTION (RegisteredApplications, APPID);
TABLE_COLLECTION (TextStyles, STYLE);
TABLE_COLLECTION (UCSs, UCS);
TABLE_COLLECTION (Viewports, VPORT);
TABLE_COLLECTION (Views, VIEW);
#undef TABLE_COLLECTION

#define DICT_COLLECTION(token,key) CLASSDESC(token)
DICT_COLLECTION (Dictionaries, NAMED_OBJECT_DICTIONARY);
//DICT_COLLECTION (PlotConfigurations, PLOTSTYLENAME);
DICT_COLLECTION (Groups, GROUP);
DICT_COLLECTION (Colors, COLOR);
DICT_COLLECTION (Layouts, LAYOUT);
DICT_COLLECTION (MlineStyles, MLINESTYLE);
DICT_COLLECTION (MLeaderStyles, MLEADERSTYLE);
DICT_COLLECTION (Materials, MATERIAL);
DICT_COLLECTION (PlotStyles, PERSUBENTMGR);
DICT_COLLECTION (DetailViewStyles, DETAILVIEWSTYLE);
DICT_COLLECTION (SectionViewStyles, SECTIONVIEWSTYLE);
DICT_COLLECTION (VisualStyles, VISUALSTYLE);
DICT_COLLECTION (Scales, SCALELIST);
DICT_COLLECTION (TableStyles, TABLESTYLE);
DICT_COLLECTION (WipeoutVars, WIPEOUT_VARS);
DICT_COLLECTION (AssocNetworks, ASSOCNETWORK);
DICT_COLLECTION (AssocPersSubentManagers, ASSOCPERSSUBENTMANAGER);
#undef DICT_COLLECTION

#define DWG_OBJECT(token) extern GB_DESC token##_Desc[];
#define DWG_ENTITY(token) DWG_OBJECT(token)
#include "objects.inc"
#undef DWG_OBJECT 
#undef DWG_ENTITY 

#undef CLASSDESC

#else /* _C_DWG_C */

#define THIS ((CDwgDocument *)_object)
#define THIS_DWG THIS->dwg
#define THIS_DXF ((CDxfDocument *)_object)

#endif /* _C_DWG_C */

typedef struct {
  GB_BASE ob;
  Dwg_Data *dwg;
  bool is_dxf;
} CDwgDocument;
typedef CDwgDocument CDxfDocument;

typedef struct {
  GB_BASE ob;
  Dwg_Data *dwg;
} CDwg;
typedef CDwg CHeader;
typedef CDwg CSummaryInfo;

typedef union {
  BITCODE_B b;
  BITCODE_RC rc;
  BITCODE_BS bs;
  BITCODE_BL bl;
  BITCODE_BLL bll;
  BITCODE_T t;
  BITCODE_H h;
  BITCODE_3BD pt;
} CDwg_Variant;

// get, put. next
#define TABLE_COLLECTION(token, object)            \
  typedef struct {                                 \
    GB_BASE ob;                                    \
    Dwg_Data *dwg;                                 \
    Dwg_Object *ctrl;                              \
    /* list or array? */                           \
    Dwg_Object_##object##_CONTROL *_ctrl;          \
  } C##token
// The nodkey: "ACAD_"#nodkey
#define DICT_COLLECTION(token, nodkey, object)     \
  typedef struct {                                 \
    GB_BASE ob;                                    \
    Dwg_Data *dwg;                                 \
    Dwg_Object *dictobj;                           \
    const char *nodkey;                            \
    /* map */                                      \
    Dwg_Object_DICTIONARY *dict;                   \
    unsigned iter;                                 \
  } C##token
#define DICT_COLLECTION2(token, object)            \
  DICT_COLLECTION(token, object, object)

TABLE_COLLECTION (DimStyles, DIMSTYLE);
TABLE_COLLECTION (Layers, LAYER);
TABLE_COLLECTION (Linetypes, LTYPE);
TABLE_COLLECTION (RegisteredApplications, APPID);
TABLE_COLLECTION (TextStyles, STYLE);
TABLE_COLLECTION (UCSs, UCS);
TABLE_COLLECTION (Viewports, VPORT);
TABLE_COLLECTION (Views, VIEW);

DICT_COLLECTION (Dictionaries, NAMED_OBJECT_DICTIONARY, DICTIONARY);
//DICT_COLLECTION (PlotConfigurations, PLOTSTYLENAME, PLOTSTYLE);
DICT_COLLECTION2 (Groups, GROUP);
DICT_COLLECTION (Colors, COLOR, DBCOLOR);
DICT_COLLECTION2 (Layouts, LAYOUT);
DICT_COLLECTION2 (MlineStyles, MLINESTYLE);
DICT_COLLECTION2 (MLeaderStyles, MLEADERSTYLE);
DICT_COLLECTION2 (Materials, MATERIAL);
DICT_COLLECTION2 (DetailViewStyles, DETAILVIEWSTYLE);
DICT_COLLECTION2 (SectionViewStyles, SECTIONVIEWSTYLE);
DICT_COLLECTION2 (VisualStyles, VISUALSTYLE);
DICT_COLLECTION (Scales, SCALELIST, SCALE);
DICT_COLLECTION2 (TableStyles, TABLESTYLE);
DICT_COLLECTION (WipeoutVars, WIPEOUT_VARS, WIPEOUTVARIABLES);
DICT_COLLECTION2 (AssocNetworks, ASSOCNETWORK);
DICT_COLLECTION2 (PersSubentManagers, PERSUBENTMGR);
DICT_COLLECTION2 (AssocPersSubentManagers, ASSOCPERSSUBENTMANAGER);

#undef ENTITY_COLLECTION
#undef TABLE_COLLECTION
#undef DICT_COLLECTION
#undef DICT_COLLECTION2

#define DWG_OBJECT(token)                       \
  typedef struct {                              \
    GB_BASE ob;                                 \
    Dwg_Data *dwg;                              \
    Dwg_Object *obj;                            \
    Dwg_Object_Object *common;                  \
    Dwg_Object_##token *_obj;                   \
    unsigned iter;                              \
  } C##token;
#define DWG_ENTITY(token)                       \
  typedef struct {                              \
    GB_BASE ob;                                 \
    Dwg_Data *dwg;                              \
    Dwg_Object *obj;                            \
    Dwg_Object_Entity *common;                  \
    Dwg_Entity_##token *_obj;                   \
    unsigned iter;                              \
  } C##token;
#include "objects.inc"
#undef DWG_OBJECT
#undef DWG_ENTITY

typedef CDICTIONARY CDwgObject;
typedef CPOINT CDwgEntity;

void dynapi_to_gb_value (const Dwg_Data *dwg,
                         const Dwg_DYNAPI_field *f,
                         const CDwg_Variant *input);
bool gb_to_dynapi_value (const Dwg_Data *dwg,
                         const GB_VALUE *input,
                         CDwg_Variant *output);
CDwgObject* obj_to_gb (Dwg_Object *obj);
CDwgObject* obj_generic_to_gb (void *_obj);
CDwgObject* handle_to_gb (Dwg_Data *dwg, Dwg_Object_Ref *hdl);
GB_DATE* TIMERLL_to_Date (BITCODE_TIMERLL date);
char* TU_to_utf8 (BITCODE_TU wstr);

#define strEQ(s1, s2) !strcmp ((s1), (s2))
#define strNE(s1, s2) strcmp ((s1), (s2))
#define strEQc(s1, s2) !strcmp ((s1), s2 "")
#define strNEc(s1, s2) strcmp ((s1), s2 "")

#define memBEGIN(s1, s2, len) (strlen (s1) >= len && !memcmp (s1, s2, len))
#define memBEGINc(s1, s2)                       \
  (strlen (s1) >= sizeof (s2 "") - 1 && !memcmp (s1, s2, sizeof (s2 "") - 1))

#define SET_PT(tgt, arg)                                   \
  if (GB.Array.Count((GB_ARRAY)VARG(arg)) % 3) {           \
    GB.Error("3DPoint &1 must have 3 floats, but has &2", #arg,\
             GB.Array.Count((GB_ARRAY)VARG(arg)));             \
    GB.ReturnVariant (NULL);                               \
    return;                                                \
  }                                                        \
  tgt.x = *(double*)GB.Array.Get ((GB_ARRAY)VARG(arg), 0); \
  tgt.y = *(double*)GB.Array.Get ((GB_ARRAY)VARG(arg), 1); \
  tgt.z = *(double*)GB.Array.Get ((GB_ARRAY)VARG(arg), 2)
#define SET_PT1(arg) \
  if (GB.Array.Count((GB_ARRAY)VARG(arg)) % 3) {           \
    GB.Error("3DPoint &1 must have 3 floats, but has &2", #arg,\
             GB.Array.Count((GB_ARRAY)VARG(arg)));             \
    GB.ReturnVariant (NULL);                               \
    return;                                                \
  }                                                        \
  arg.x = *(double*)GB.Array.Get ((GB_ARRAY)VARG(arg), 0); \
  arg.y = *(double*)GB.Array.Get ((GB_ARRAY)VARG(arg), 1); \
  arg.z = *(double*)GB.Array.Get ((GB_ARRAY)VARG(arg), 2)
#define SET_PT2D(arg)                                      \
  if (GB.Array.Count((GB_ARRAY)VARG(arg)) % 2) {           \
    GB.Error("2DPoint &1 must have 2 floats, but has &2", #arg,\
             GB.Array.Count((GB_ARRAY)VARG(arg)));             \
    GB.ReturnVariant (NULL);                               \
    return;                                                \
  }                                                        \
  arg.x = *(double*)GB.Array.Get ((GB_ARRAY)VARG(arg), 0); \
  arg.y = *(double*)GB.Array.Get ((GB_ARRAY)VARG(arg), 1)

#endif
