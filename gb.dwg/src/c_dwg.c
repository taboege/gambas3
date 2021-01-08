/***************************************************************************

  c_dwg.c

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

#define _C_DWG_C

#include "c_dwg.h"
//#include "gb_common.h"
#include <stdbool.h>

//#include <stdio.h>
//#include <stdint.h>
//#include <math.h>

/**********************************************************************
 Gambas Interface
***********************************************************************/

// returns a field value
void dynapi_to_gb_value (const Dwg_Data *dwg,
                         const Dwg_DYNAPI_field *f,
                         const CDwg_Variant *input)
{
  if (!f) {
    fprintf (stderr, "Unknown field");
    GB.Error (GB_ERR_BOUND);
    GB.ReturnVariant (NULL);
    return;
  }
  switch (f->size) {
  case 1:
    if (strEQc (f->type, "B")) {
      GB.ReturnBoolean (input->b);
      return;
    }
    if (strEQc (f->type, "RC") || strEQc (f->type, "BB")) {
      //GB.ReturnByte (input->rc);
      GB.ReturnInteger (input->rc);
      return;
    }
  case 2:
    if (strEQc (f->type, "BS") || strEQc (f->type, "RS")) {
      //GB.ReturnShort (input->bs);
      GB.ReturnInteger (input->bs);
      return;
    }
  case 4:
    if (strEQc (f->type, "BL") || strEQc (f->type, "RL")) {
      GB.ReturnInteger (input->bl);
      return;
    }
  case 8:
    if (strEQc (f->type, "BLL") || strEQc (f->type, "RLL")) {
      GB.ReturnLong (input->bll);
      return;
    }
    // FIXME TF -> Array_Byte_FromString
    if (*f->type == 'T') {
      GB.ReturnString (input->t);
      //value->type = GB_T_CSTRING;
      //value->value._string = input->t;
      return;
    }
    if (*f->type == 'H') {
      //value->type = GB_T_LONG;
      // TODO OBJECT of type DwgHandle
      //value->value._long = input->h->absolute_ref;
      GB.ReturnLong (input->h->absolute_ref);
      return;
    }
    break;
  case 16:
  case 24: // Float[] or NATIVE_FloatArray
    {
      GB_ARRAY floats;
      if (f->size < 16) {
        break;
      }
      GB.Array.New (POINTER(&floats), GB_T_FLOAT, f->size == 16 ? 2 : 3);
      *(double*)GB.Array.Get (floats, 0) = input->pt.x;
      *(double*)GB.Array.Get (floats, 1) = input->pt.y;
      if (f->size == 24)
        *(double*)GB.Array.Get (floats, 2) = input->pt.z;
      GB.ReturnObject (floats);
      return;
    }
  default:
    break;
  }
  fprintf (stderr, "Unhandled type %s", f->type);
  GB.Error (GB_ERR_TYPE);
  GB.ReturnVariant (NULL);
  return;
}

// sets a field value
bool gb_to_dynapi_value (const Dwg_Data *dwg,
                         const GB_VALUE *input,
                         CDwg_Variant *output)
{
  GB.Error (GB_ERR_TYPE);
  return false;
}

// returns generic Object
CDwgObject* obj_to_gb (Dwg_Object *obj)
{
  GB_CLASS klass = GB.FindClass(obj->dxfname); // ? or its VBA name?
  CDwgObject *gb = (CDwgObject *)GB.New(klass, NULL, NULL);
  gb->dwg = obj->parent;
  gb->obj = obj;
  if (obj->supertype == DWG_SUPERTYPE_ENTITY)
    gb->common = (Dwg_Object_Object*)obj->tio.entity;
  else
    gb->common = obj->tio.object;
  return gb;
}

// returns generic Object
CDwgObject* obj_generic_to_gb (void *_obj)
{
  int error;
  Dwg_Object *obj = dwg_obj_generic_to_object (_obj, &error);
  if (error)
    return NULL;
  else
    return obj_to_gb (obj);
}

// returns generic Object
CDwgObject* handle_to_gb (Dwg_Data *dwg, Dwg_Object_Ref *hdl)
{
  Dwg_Object *obj = dwg_ref_object (dwg, hdl);
  if (!obj)
    return NULL;
  else
    return obj_to_gb (obj);
}

GB_DATE* TIMERLL_to_Date (BITCODE_TIMERLL date)
{
  GB.Error ("Not yet implemented");
  GB.ReturnVariant (NULL);
}

char* TU_to_utf8 (BITCODE_TU wstr)
{
  GB.Error ("Not yet implemented");
  GB.ReturnString (NULL);
}

BEGIN_METHOD (DwgDocument_new, GB_STRING file;) /* optional */
  char *file = STRING (file);
  CDwgDocument *cdwg = GB.New(GB.FindClass("DwgDocument"), NULL, NULL);
  dwg_read_file (file, cdwg->dwg);
  GB.ReturnObject (cdwg);
END_METHOD

BEGIN_METHOD (DwgDocument_Open, GB_STRING file;)
  char *file = STRING (file);
  CDwgDocument *cdwg = GB.New(GB.FindClass("DwgDocument"), NULL, NULL);
  dwg_read_file (file, cdwg->dwg); // TODO error handling
  // TODO: create all the properties and collections at init or on the fly?
  GB.ReturnObject (cdwg);
END_METHOD

BEGIN_METHOD (DwgDocument_Add, GB_STRING version;) /* optional: is_imperial */
  char *version = STRING (version);
  Dwg_Version_Type ver = dwg_version_as (version);
  GB_CLASS klass = GB.FindClass("DwgDocument");
  CDwgDocument *cdwg = GB.New(klass, NULL, NULL);
  if (ver == R_INVALID || ver == R_AFTER) {
    // Invalid version argument
    GB.Error (GB_ERR_TYPE);
    return;
  }
  cdwg->dwg = dwg_add_Document (ver, false, 0);
  GB.ReturnObject (cdwg);
END_METHOD

BEGIN_METHOD (DwgDocument_Save, GB_STRING file;)
  char *file = STRING (file);
  dwg_write_file (file, THIS_DWG); // TODO error handling
END_METHOD

BEGIN_METHOD_VOID (DwgDocument_free)
  dwg_free (THIS_DWG);
END_METHOD

BEGIN_PROPERTY(ModelSpace_prop)
  const Dwg_Data *dwg = THIS_DWG;
  GB.ReturnObject (obj_to_gb (dwg->mspace_block));
END_PROPERTY

BEGIN_PROPERTY(PaperSpace_prop)
  const Dwg_Data *dwg = THIS_DWG;
  GB.ReturnObject (obj_to_gb (dwg->pspace_block));
END_PROPERTY

BEGIN_PROPERTY(SummaryInfo_prop)
  Dwg_Data *dwg = THIS_DWG;
  CSummaryInfo *csi = GB.New(GB.FindClass("SummaryInfo"), NULL, NULL);
  csi->dwg = dwg;
  GB.ReturnObject (csi);
END_PROPERTY

BEGIN_PROPERTY(Blocks_prop)
  const Dwg_Data *dwg = THIS_DWG;
  GB.ReturnObject (obj_generic_to_gb (&dwg->block_control));
END_PROPERTY

BEGIN_PROPERTY(Layers_prop)
  const Dwg_Data *dwg = THIS_DWG;
  GB.ReturnObject (obj_generic_to_gb (&dwg->layer_control));
END_PROPERTY

BEGIN_PROPERTY(TextStyles_prop)
  const Dwg_Data *dwg = THIS_DWG;
  GB.ReturnObject (obj_generic_to_gb (&dwg->style_control));
END_PROPERTY

BEGIN_PROPERTY(Linetypes_prop)
  const Dwg_Data *dwg = THIS_DWG;
  GB.ReturnObject (obj_generic_to_gb (&dwg->ltype_control));
END_PROPERTY

BEGIN_PROPERTY(Regapps_prop)
  const Dwg_Data *dwg = THIS_DWG;
  GB.ReturnObject (obj_generic_to_gb (&dwg->appid_control));
END_PROPERTY

BEGIN_PROPERTY(DimStyles_prop)
  const Dwg_Data *dwg = THIS_DWG;
  GB.ReturnObject (obj_generic_to_gb (&dwg->dimstyle_control));
END_PROPERTY

BEGIN_PROPERTY(UCSs_prop)
  const Dwg_Data *dwg = THIS_DWG;
  GB.ReturnObject (obj_generic_to_gb (&dwg->ucs_control));
END_PROPERTY

BEGIN_PROPERTY(Viewports_prop)
  const Dwg_Data *dwg = THIS_DWG;
  GB.ReturnObject (obj_generic_to_gb (&dwg->vport_control));
END_PROPERTY

BEGIN_PROPERTY(Views_prop)
  const Dwg_Data *dwg = THIS_DWG;
  GB.ReturnObject (obj_generic_to_gb (&dwg->view_control));
END_PROPERTY

GB_DESC DwgDocument_Desc[] =
{
  GB_DECLARE("DwgDocument", sizeof(CDwgDocument)),

  // Versions
  GB_CONSTANT("R_INVALID","i", R_INVALID),
  GB_CONSTANT("R_1_1","i", R_1_1),	/* MC0.0  MicroCAD Release 1.1 */
  GB_CONSTANT("R_1_2","i", R_1_2),	/* AC1.2  AutoCAD Release 1.2 */
  GB_CONSTANT("R_1_3","i", R_1_3),	/* AC1.3  AutoCAD Release 1.3 */
  GB_CONSTANT("R_1_4","i", R_1_4),	/* AC1.40 AutoCAD Release 1.4 */
  GB_CONSTANT("R_1_402b","i", R_1_402b),/* AC1.402b AutoCAD Release 1.402b */
  GB_CONSTANT("R_2_0","i", R_2_0),	/* AC1.50 AutoCAD Release 2.0 */
  GB_CONSTANT("R_2_1","i", R_2_1),	/* AC2.10 AutoCAD Release 2.10 */
  GB_CONSTANT("R_2_21","i", R_2_21),	/* AC2.21 AutoCAD Release 2.21 */
  GB_CONSTANT("R_2_22","i", R_2_22),	/* AC2.22 AutoCAD Release 2.22 */
  GB_CONSTANT("R_2_4","i", R_2_4),	/* AC1001 AutoCAD Release 2.4 */
  GB_CONSTANT("R_2_5","i", R_2_5),	/* AC1002 AutoCAD Release 2.5 */
  GB_CONSTANT("R_2_6","i", R_2_6),	/* AC1003 AutoCAD Release 2.6 */
  GB_CONSTANT("R_9","i", R_9),		/* AC1004 AutoCAD Release 9 */
  GB_CONSTANT("R_10","i", R_10),	/* AC1006 AutoCAD Release 10 */
  GB_CONSTANT("R_10c1","i", R_10c1),	/* AC1007 AutoCAD Release 10c1 */
  GB_CONSTANT("R_10c2","i", R_10c2),	/* AC1008 AutoCAD Release 10c2 */
  GB_CONSTANT("R_11","i", R_11),	/* AC1009 AutoCAD Release 11/12 (LT R1/R2) */
  GB_CONSTANT("R_12","i", R_12),	/* AC1010 AutoCAD Release 12 */
  GB_CONSTANT("R_12c1","i", R_12c1),	/* AC1011 AutoCAD Release 12c1 */
  GB_CONSTANT("R_13","i", R_13),	/* AC1012 AutoCAD Release 13 */
  GB_CONSTANT("R_13c3","i", R_13c3),	/* AC1013 AutoCAD Release 13C3 */
  GB_CONSTANT("R_14","i", R_14),	/* AC1014 AutoCAD Release 14 */
  GB_CONSTANT("R_2000","i", R_2000),	/* AC1015 AutoCAD Release 2000 */
  GB_CONSTANT("R_2004","i", R_2004),	/* AC1018 AutoCAD Release 2004-2006
                                           (includes versions AC1019/0x19 and AC1020/0x1a) */
  GB_CONSTANT("R_2007","i", R_2007),	/* AC1021 AutoCAD Release 2007-2009 */
  GB_CONSTANT("R_2010","i", R_2010),	/* AC1024 AutoCAD Release 2010-2012 */
  GB_CONSTANT("R_2013","i", R_2013),	/* AC1027 AutoCAD Release 2013-2017 */
  GB_CONSTANT("R_2018","i", R_2018),	/* AC1032 AutoCAD Release 2018-2021 */
  GB_CONSTANT("R_AFTER","i", R_AFTER),   // also invalid

  GB_METHOD("_new",  "o", DwgDocument_new, "[(File)]"),
  GB_METHOD("_free", 0,   DwgDocument_free, 0),
  GB_METHOD("Add",   "o", DwgDocument_Add, "[(Version)]"),
  GB_METHOD("Open",  "o", DwgDocument_Open,"File"),
  GB_METHOD("Save",  0,   DwgDocument_Save,"(File)"),
  /*
  GB_METHOD("SaveAs",0, DwgDocument_SaveAs,"[File, (Version)]"),
  GB_METHOD("Export",0, DwgDocument_Export,"[File, Extension ]"),
  GB_METHOD("Close", 0, DwgDocument_Close, 0),

  GB_METHOD("AddDictionary", 0, Dwg_AddDictionary, "(Name)s(Key)s(Handle)o"),
  GB_METHOD("AddDictionaryWDflt", 0, Dwg_AddDictionaryWDflt, "(Name)s(Key)s(Handle)o"),
  GB_METHOD("AddPlaceholder", 0, Dwg_AddPlaceholder, 0),

  GB_METHOD("AddLayer",       0, Dwg_AddLayer, "(Name)s"),
  GB_METHOD("AddLinetype",    0, Dwg_AddLinetype, "(Name)s"),
  GB_METHOD("AddRegisteredApplication", 0, Dwg_AddRegapp, "(Name)s"),
  GB_METHOD("AddTextStyle","o", Dwg_AddTextStyle, "(Name)s"),
  GB_METHOD("AddDimStyle","o", Dwg_AddDimStyle, "(Name)s"),
  GB_METHOD("AddUCS","o", Dwg_AddUCS, "(Name)s(Origin)a(XAxis)a(YAxis)a"),
  GB_METHOD("AddViewport","o", Dwg_AddViewport, "(Name)s"),
  GB_METHOD("AddView","o", Dwg_AddView, "(Name)s"),
  GB_METHOD("AddGroup","o", Dwg_AddView, "(Name)s"),
  EndUndoMark
  */

  GB_PROPERTY_READ("ModelSpace","o", ModelSpace_prop),
  GB_PROPERTY_READ("PaperSpace","o", PaperSpace_prop),
  GB_PROPERTY_READ("Blocks","o", Blocks_prop),
  GB_PROPERTY_READ("SummaryInfo","o", SummaryInfo_prop),

  GB_PROPERTY_READ("Layers","o", Blocks_prop),
  GB_PROPERTY_READ("Linetypes","o", Linetypes_prop),
  GB_PROPERTY_READ("RegisteredApplications","o", Regapps_prop),
  GB_PROPERTY_READ("TextStyles","o", TextStyles_prop),
  GB_PROPERTY_READ("DimStyles","o", DimStyles_prop),
  GB_PROPERTY_READ("UCSs","o", UCSs_prop),
  GB_PROPERTY_READ("Viewports","o", Viewports_prop),
  GB_PROPERTY_READ("Views","o", Views_prop),

  GB_END_DECLARE
};

// This is mostly the same as the DwgDocument, just the importers and exporters are different.
// It really should create a DwgDocument, and just provide some methods.
GB_DESC DxfDocument_Desc[] =
{
  GB_DECLARE("DxfDocument", sizeof(CDxfDocument)), GB_INHERITS("DwgDocument"),
  /*
  GB_METHOD("_new",  0, DxfDocument_new, "[(File)]"),
  GB_METHOD("_free", 0, DxfDocument_free, 0),
  GB_METHOD("Open",  0, DxfDocument_open, "File"),
  GB_METHOD("Save",  0, DxfDocument_save, "(File)"),
  GB_METHOD("SaveAs",0, DxfDocument_saveas, "[File, (Version)]"),
  GB_METHOD("Close", 0, DxfDocument_close, 0),
  */
  GB_END_DECLARE
};

BEGIN_PROPERTY(SummaryInfo_title)
  const Dwg_Data *dwg = THIS_DWG;
  const Dwg_SummaryInfo *si = &dwg->summaryinfo;
  GB.ReturnString ( TU_to_utf8 (si->TITLE));
END_PROPERTY

BEGIN_PROPERTY(SummaryInfo_subject)
  const Dwg_Data *dwg = THIS_DWG;
  const Dwg_SummaryInfo *si = &dwg->summaryinfo;
  GB.ReturnString ( TU_to_utf8 (si->SUBJECT));
END_PROPERTY

BEGIN_PROPERTY(SummaryInfo_author)
  const Dwg_Data *dwg = THIS_DWG;
  const Dwg_SummaryInfo *si = &dwg->summaryinfo;
  GB.ReturnString ( TU_to_utf8 (si->AUTHOR));
END_PROPERTY

BEGIN_PROPERTY(SummaryInfo_keywords)
  const Dwg_Data *dwg = THIS_DWG;
  const Dwg_SummaryInfo *si = &dwg->summaryinfo;
  GB.ReturnString ( TU_to_utf8 (si->KEYWORDS));
END_PROPERTY

BEGIN_PROPERTY(SummaryInfo_comments)
  const Dwg_Data *dwg = THIS_DWG;
  const Dwg_SummaryInfo *si = &dwg->summaryinfo;
  GB.ReturnString ( TU_to_utf8 (si->COMMENTS));
END_PROPERTY

BEGIN_PROPERTY(SummaryInfo_tdindwg)
  const Dwg_Data *dwg = THIS_DWG;
  const Dwg_SummaryInfo *si = &dwg->summaryinfo;
  GB.ReturnDate ( TIMERLL_to_Date (si->TDINDWG) );
END_PROPERTY

BEGIN_PROPERTY(SummaryInfo_tdcreate)
  const Dwg_Data *dwg = THIS_DWG;
  const Dwg_SummaryInfo *si = &dwg->summaryinfo;
  GB.ReturnDate ( TIMERLL_to_Date (si->TDCREATE) );
END_PROPERTY

BEGIN_PROPERTY(SummaryInfo_tdupdate)
  const Dwg_Data *dwg = THIS_DWG;
  const Dwg_SummaryInfo *si = &dwg->summaryinfo;
  GB.ReturnDate ( TIMERLL_to_Date (si->TDUPDATE) );
END_PROPERTY

GB_DESC SummaryInfo_Desc[] =
{
   GB_DECLARE("SummaryInfo", 0), GB_NOT_CREATABLE(),
   GB_PROPERTY_READ("Title","s",SummaryInfo_title),
   GB_PROPERTY_READ("Author","s",SummaryInfo_author),
   GB_PROPERTY_READ("Subject","s",SummaryInfo_subject),
   GB_PROPERTY_READ("Keywords","s",SummaryInfo_keywords),
   GB_PROPERTY_READ("Comments","s",SummaryInfo_comments),
   GB_PROPERTY_READ("TimeInDwg","d",SummaryInfo_tdindwg),
   GB_PROPERTY_READ("CreationDate","d",SummaryInfo_tdcreate),
   GB_PROPERTY_READ("ModificationDate","d",SummaryInfo_tdupdate),
   // GetCustomByIndex
   // GetCustomByKey
   GB_END_DECLARE
};

#define DWG (((CDwg *)_object)->dwg)

BEGIN_METHOD (Header_get, GB_STRING name;)

  char *key = GB.ToZeroString(ARG(name));
  const Dwg_Data *dwg = THIS_DWG;
  CDwg_Variant value;
  Dwg_DYNAPI_field f;
  GB_VARIANT retval;

  if (!dwg_dynapi_header_value (dwg, key, &value, &f))
    {
      GB.Error (GB_ERR_BOUND);
      return;
    }
  dynapi_to_gb_value (dwg, &f, &value);
  GB.ReturnConvVariant ();

END_METHOD

BEGIN_METHOD (Header_put, GB_VARIANT value; GB_STRING name;)

  char *key = GB.ToZeroString(ARG(name));
  GB_VALUE *value = (GB_VALUE *)ARG(value);
  const Dwg_Data *dwg = THIS_DWG;
  CDwg_Variant out;

  if (!gb_to_dynapi_value (dwg, value, &out))
    GB.Error (GB_ERR_TYPE);
  else
    if (!dwg_dynapi_header_set_value (dwg, key, &out, true))
      GB.Error (GB_ERR_BOUND);

END_METHOD

GB_DESC Header_Desc[] =
{
   GB_DECLARE("Header", 0), GB_NOT_CREATABLE(),
   // Just declare the getter and setter method via dynapi.
   // No properties, no autocompletion on the header_vars fields
   GB_METHOD("_get", "v", Header_get, "(Name)s"),
   GB_METHOD("_put", NULL, Header_put, "(Value)v(Name)s"),
   GB_END_DECLARE
};

#undef THIS
#define THIS ((CDimStyles*)_object)

BEGIN_PROPERTY(Objects_Count)

  // FIXME number of entities in this collection
  Dwg_Data *dwg = THIS->dwg;
  GB.ReturnInteger(dwg->num_objects);

END_PROPERTY

BEGIN_METHOD (Objects_get, GB_INTEGER index;)

  Dwg_Data *dwg = THIS->dwg;
  unsigned index = (unsigned)VARG(index);
  Dwg_Object *obj;
  if (index >= dwg->num_objects) 
    {
      GB.Error (GB_ERR_BOUND);
      return;
    }
  obj = &dwg->object[index];
  // convert to Object/Entity/Table/Dict depending on type
  GB.ReturnObject (obj_to_gb (obj));

END_METHOD

// This is backed by the dwg->object[] array. By Index. Returns Dwg_Object_OBJECT*
#define OBJECTS_ARRAY(token)                                    \
GB_DESC token##_Desc[] =                                        \
  {                                                             \
    GB_DECLARE(#token, sizeof(C##token)), GB_NOT_CREATABLE(),   \
    /* Array of objects */                                      \
    GB_PROPERTY_READ("Count", "i", Objects_count),              \
    /*GB_METHOD("_new", "v", Objects_add, "(Type)s"),*/         \
    GB_METHOD("_get", "v", Objects_get, "(Index)i"),            \
    /*GB_METHOD("_put", NULL,Objects_put, "(Object)v(Index)i"),*/ \
    GB_END_DECLARE                                              \
  }

#undef THIS
#define THIS ((CDimStyles*)_object)

BEGIN_PROPERTY(Table_Count)
  GB.ReturnInteger(THIS->_ctrl->num_entries);
END_PROPERTY

// Add Tablerecord to Tables
BEGIN_METHOD (Table_Add, GB_STRING name)
  char *name = STRING (name);
  Dwg_Object_Type type = THIS->ctrl->fixedtype;

#define ADD_TABLE(token)                                 \
  if (type == DWG_TYPE_##token)                          \
    {                                                    \
      Dwg_Object_##token *_obj = dwg_add_##token (THIS->dwg, name); \
      GB.ReturnObject (obj_generic_to_gb (_obj));        \
    }
  ADD_TABLE (BLOCK_HEADER) else
  ADD_TABLE (LAYER) else
  ADD_TABLE (LTYPE) else
  ADD_TABLE (DIMSTYLE) else
  ADD_TABLE (APPID) else
  ADD_TABLE (STYLE) else
  ADD_TABLE (VPORT) else
  ADD_TABLE (VIEW) else
  if (type == DWG_TYPE_UCS) {
    dwg_point_3d origin = {0.0,0.0,0.0}, x_axis = {1.0,0.0,0.0}, y_axis = {0.0,1.0,0.0};
    Dwg_Object_UCS *_obj = dwg_add_UCS (THIS->dwg, &origin, &x_axis, &y_axis, name);
    GB.ReturnObject (obj_generic_to_gb (_obj));
  }
  else {
    GB.Error (GB_ERR_TYPE);
    return;
  }
END_METHOD

// lookup in TABLE_CONTROL
BEGIN_METHOD (Table_get_by_index, GB_INTEGER index)
  unsigned index = VARG(index);
  if (index >= THIS->_ctrl->num_entries)
    {
      GB.Error (GB_ERR_BOUND);
      return;
    }
  GB.ReturnObject (handle_to_gb (THIS->dwg, THIS->_ctrl->entries[index]));
END_METHOD

BEGIN_METHOD (Table_get_by_name, GB_STRING name)
  char *name = STRING (name);
  for (unsigned i = 0; i < THIS->_ctrl->num_entries; i++)
    {
      Dwg_Object_Ref *ref = THIS->_ctrl->entries[i];
      Dwg_Object *obj = dwg_ref_object (THIS->dwg, ref);
      Dwg_Object_DIMSTYLE *_obj = obj->tio.object->tio.DIMSTYLE;
      if (strEQ (_obj->name, name)) {
        GB.ReturnObject (obj_to_gb (obj));
        return;
      }
    }
  GB.ReturnVariant (NULL);
END_METHOD

// This is backed by an TABLE_CONTROL array, looked up by name or index.
// Returns Dwg_Object_OBJECT*
#define TABLE_ARRAY(token, obj)                               \
GB_DESC token##s_Desc[] =                                     \
  {                                                           \
    GB_DECLARE(#token"s", sizeof(C##token##s)), GB_NOT_CREATABLE(), \
    /* Array of table records */                              \
    GB_PROPERTY_READ("Count", "i", Table_Count),              \
    GB_METHOD("Add", "v", Table_Add, "(Name)s"),              \
    GB_METHOD("_get", "v", Table_get_by_index, "(Index)i"),   \
    GB_METHOD("_get", "v", Table_get_by_name, "(Name)s"),     \
    /*GB_METHOD("_put", NULL,Table_put, "(Object)v(Index)i"),*/\
    /*GB_METHOD("_next", "v",Table_next, NULL),*/             \
    GB_END_DECLARE                                            \
  }
//TABLE_ARRAY (Block, BLOCK_HEADER);
TABLE_ARRAY (DimStyle, DIMSTYLE);
TABLE_ARRAY (Layer, LAYER);
TABLE_ARRAY (Linetype, LTYPE);
TABLE_ARRAY (RegisteredApplication, APPID);
TABLE_ARRAY (TextStyle, STYLE);
TABLE_ARRAY (UCS, UCS);
TABLE_ARRAY (Viewport, VPORT);
TABLE_ARRAY (View, VIEW);

#undef THIS
#define THIS ((CDictionaries*)_object)

BEGIN_PROPERTY(Dict_Count)
  GB.ReturnInteger(THIS->dict->numitems);
END_PROPERTY

BEGIN_METHOD (Dict_get, GB_STRING key)
  Dwg_Object_DICTIONARY *dict = THIS->dict;
  char *key = STRING (key);
  for (unsigned i = 0; i < dict->numitems; i++)
    {
      if (strEQ (dict->texts[i], key)) {
        GB.ReturnObject (handle_to_gb (THIS->dwg, dict->itemhandles[i]));
        return;
      }
    }
  GB.ReturnVariant (NULL);
END_METHOD

BEGIN_METHOD (Dict_put, GB_OBJECT obj; GB_STRING key)
  // FIXME
  GB.Error ("Not yet implemented");
  GB.ReturnVariant (NULL);
END_METHOD

BEGIN_METHOD_VOID (Dict_next)
  Dwg_Object_DICTIONARY *dict = THIS->dict;
  if (THIS->iter >= dict->numitems)
    GB.StopEnum();
  else
    GB.ReturnObject (handle_to_gb (THIS->dwg, dict->itemhandles[THIS->iter++]));
END_METHOD

// Lookup by name in the dictionary array. Returns a dwg_obj_generic or handle
#define DICT_COLLECTION(token, nodkey, obj)                   \
GB_DESC token##_Desc[] =                                      \
  {                                                           \
    GB_DECLARE(#token, sizeof(C##token)), GB_NOT_CREATABLE(), \
    /* Hash of dict items */                                  \
    GB_PROPERTY_READ("Count", "i", Dict_Count),               \
    GB_METHOD("_get", "o", Dict_get, "(Key)s"),               \
    GB_METHOD("_put", NULL,Dict_put, "(Object)o(Key)s"),      \
    GB_METHOD("_next", "o",Dict_next, NULL),                  \
    GB_END_DECLARE                                            \
  }
#define DICT_COLLECTION2(token, obj) DICT_COLLECTION(token, obj, obj)

DICT_COLLECTION (Dictionaries, NAMED_OBJECT_DICTIONARY, DICTIONARY);
//DICT_COLLECTION (PlotConfigurations, PLOTSTYLENAME, PLOTSTYLE);
DICT_COLLECTION2 (Groups, GROUP);
DICT_COLLECTION (Colors, COLOR, DBCOLOR);
DICT_COLLECTION2 (Layouts, LAYOUT);
DICT_COLLECTION2 (MlineStyles, MLINESTYLE);
DICT_COLLECTION2 (MLeaderStyles, MLEADERSTYLE);
DICT_COLLECTION2 (Materials, MATERIAL);
//DICT_COLLECTION2 (PlotStyles, PLOTSTYLE);
DICT_COLLECTION2 (DetailViewStyles, DETAILVIEWSTYLE);
DICT_COLLECTION2 (SectionViewStyles, SECTIONVIEWSTYLE);
DICT_COLLECTION2 (VisualStyles, VISUALSTYLE);
DICT_COLLECTION (Scales, SCALELIST, SCALE);
DICT_COLLECTION2 (TableStyles, TABLESTYLE);
DICT_COLLECTION (WipeoutVars, WIPEOUT_VARS, WIPEOUTVARIABLES);
DICT_COLLECTION2 (AssocNetworks, ASSOCNETWORK);
DICT_COLLECTION2 (PersSubentManagers, PERSUBENTMGR);
DICT_COLLECTION2 (AssocPersSubentManagers, ASSOCPERSSUBENTMANAGER);
#undef TABLE_COLLECTION
#undef DICT_COLLECTION
#undef DICT_COLLECTION2

#undef THIS
#define THIS ((CXRECORD*)_object)
#define THIS_ENT ((CPOINT*)_object)


/* Object-specific methods */

BEGIN_METHOD_VOID (Insert_ConvertToAnonymousBlock)
  // TODO
  GB.Error ("Not yet implemented");
  GB.ReturnVariant (NULL);
END_METHOD

BEGIN_METHOD_VOID (Insert_GetAttributes)
  //TODO
  GB.Error ("Not yet implemented");
  GB.ReturnVariant (NULL);
END_METHOD

// TODO for all dynamic blocks (has AcDbEvalExpr?)
BEGIN_METHOD (DynBlock_ConvertToStaticBlock, GB_STRING name)
  const char *name = STRING (name);
  GB.Error ("Not yet implemented");
  GB.ReturnVariant (NULL);
END_METHOD

BEGIN_METHOD_VOID (DynBlock_GetDynamicBlockProperties)
  //TODO
  GB.Error ("Not yet implemented");
  GB.ReturnVariant (NULL);
END_METHOD

// GB_METHOD("AppendVertex", 0, PLine_AppendVertex, "(point)f[3]"),
BEGIN_METHOD (PLine_AppendVertex, GB_OBJECT point)
  dwg_point_3d point;
  SET_PT1 (point);
  GB.Error ("Not yet implemented");
  GB.ReturnVariant (NULL);
END_METHOD

// GB_METHOD("GetBulge", "f", PLine_GetBulge, "(index)i"),
BEGIN_METHOD (PLine_GetBulge, GB_INTEGER index)
  const int index = (int)VARG(index);
  Dwg_Object *obj = THIS->obj;
  if (obj->fixedtype != DWG_TYPE_LWPOLYLINE &&
      obj->fixedtype != DWG_TYPE_POLYLINE_2D) {
    GB.Error (GB_ERR_TYPE);
    GB.ReturnVariant (NULL);
    return;
  }
  GB.Error ("Not yet implemented");
  GB.ReturnFloat (0.0);
END_METHOD

#undef THIS
#define THIS ((CXRECORD*)_object)
#define THIS_ENT ((CPOINT*)_object)

BEGIN_METHOD (_3DFace_GetInvisibleEdge, GB_INTEGER index)
  const int index = (int)VARG(index);
  Dwg_Entity__3DFACE *_obj = ((C_3DFACE*)_object)->_obj;
  BITCODE_BS invis_flag = _obj->invis_flags;
  if (index < 0 || index > 3)
    GB.Error (GB_ERR_BOUND);
  GB.ReturnBoolean (invis_flag & index);
END_METHOD

BEGIN_METHOD (Spline_DeleteFitPoint, GB_INTEGER index)
  const int index = (int)VARG(index);
  Dwg_Entity_SPLINE *_obj = ((CSPLINE*)_object)->_obj;
  if (index < 0 || index >= _obj->num_fit_pts)
    GB.Error (GB_ERR_BOUND);
  // CHECKME
  if (index != _obj->num_fit_pts - 1)
    memmove (&_obj->fit_pts[index], &_obj->fit_pts[index+1], _obj->num_fit_pts - index);
  _obj->num_fit_pts--;
END_METHOD

// GB_METHOD ("ElevateOrder", 0, Spline_ElevateOrder, "(order)i"),
BEGIN_METHOD (Spline_ElevateOrder, GB_INTEGER index)
  const int index = (int)VARG(index);
  Dwg_Entity_SPLINE *_obj = ((CSPLINE*)_object)->_obj;
  if (index < 0 || index >= _obj->num_fit_pts)
    GB.Error (GB_ERR_BOUND);
  GB.Error ("Not yet implemented");
END_METHOD

// GB_METHOD ("GetControlPoint", "o", Spline_GetControlPoint, "(index)i"),
BEGIN_METHOD (Spline_GetControlPoint, GB_INTEGER index)
  const int index = (int)VARG(index);
  Dwg_Entity_SPLINE *_obj = ((CSPLINE*)_object)->_obj;
  Dwg_SPLINE_control_point pt;
  GB_ARRAY point;
  if (index < 0 || index >= _obj->num_ctrl_pts)
    GB.Error (GB_ERR_BOUND);
  pt = _obj->ctrl_pts[index];
  GB.Array.New (POINTER(&point), GB_T_FLOAT, 4);
  *(double*)GB.Array.Get (point, 0) = pt.x;
  *(double*)GB.Array.Get (point, 1) = pt.y;
  *(double*)GB.Array.Get (point, 2) = pt.z;
  *(double*)GB.Array.Get (point, 3) = pt.w;
  GB.ReturnObject (point);
END_METHOD

// GB_METHOD ("GetFitPoint", "o", Spline_GetFitPoint, "(index)i"),
BEGIN_METHOD (Spline_GetFitPoint, GB_INTEGER index)
  const int index = (int)VARG(index);
  Dwg_Entity_SPLINE *_obj = ((CSPLINE*)_object)->_obj;
  BITCODE_3DPOINT pt;
  GB_ARRAY point;
  if (index < 0 || index >= _obj->num_fit_pts)
    GB.Error (GB_ERR_BOUND);
  pt = _obj->fit_pts[index];
  // TODO RETURN_PT macro
  GB.Array.New (POINTER(&point), GB_T_FLOAT, 3);
  *(double*)GB.Array.Get (point, 0) = pt.x;
  *(double*)GB.Array.Get (point, 1) = pt.y;
  *(double*)GB.Array.Get (point, 2) = pt.z;
  GB.ReturnObject (point);
END_METHOD

/* get/set Object or Entity fields by fieldname */

BEGIN_PROPERTY(Object_fieldcount)
  const dwg_ent_generic *obj = (dwg_ent_generic *)THIS->_obj;
  const char *name = THIS->obj->name;
  Dwg_DYNAPI_field *f = dwg_dynapi_entity_fields (name);
  unsigned n = 0;
  for (; f && f->name; f++)
    n++;
  f = THIS->obj->supertype == DWG_SUPERTYPE_ENTITY
    ? dwg_dynapi_common_entity_fields ()
    : dwg_dynapi_common_object_fields ();
  for (; f && f->name; f++)
    n++;
  GB.ReturnInteger (n);
END_PROPERTY

// Objects are added to the DWG, Entities to a BLOCK_HEADER
// Not here, rather see c_ents.c
/*
BEGIN_METHOD (Object_Add, GB_OBJECT dwg; GB_STRING name;)
  const char *klass = STRING (name);
  // FIXME
  GB.Error ("Not yet implemented");
  GB.ReturnVariant (NULL);
END_METHOD
*/

BEGIN_METHOD_VOID (Object_Copy)
  // FIXME clone the object/entity
  GB.Error ("Not yet implemented");
  GB.ReturnVariant (NULL);
END_METHOD

BEGIN_METHOD_VOID (Object_Delete)
  // FIXME
  GB.Error ("Not yet implemented");
  GB.ReturnVariant (NULL);
END_METHOD

// GetExtensionDictionary", "_Dictionary;", Object_GetExtensionDictionary, NULL)
BEGIN_METHOD_VOID (Object_GetExtensionDictionary)
  // FIXME
  GB.Error ("Not yet implemented");
  GB.ReturnVariant (NULL);
END_METHOD

/*
BEGIN_METHOD (Entity_Add, GB_OBJECT blkhdr; GB_STRING name)
  const char *klass = STRING (name);
  // FIXME
  GB.Error ("Not yet implemented");
  GB.ReturnVariant (NULL);
END_METHOD
*/

BEGIN_METHOD (Object_get, GB_STRING field)

  const char *key = STRING (field);
  const dwg_ent_generic *obj = (dwg_ent_generic *)THIS->_obj;
  const char *dxfname = THIS->obj->dxfname;
  const Dwg_Data *dwg = THIS->dwg;
  CDwg_Variant value;
  Dwg_DYNAPI_field f;
  GB_VARIANT retval;

  if (!dwg_dynapi_entity_value (obj, dxfname, key, &value, &f))
    {
      GB.Error (GB_ERR_BOUND);
      return;
    }
  dynapi_to_gb_value (dwg, &f, &value);

END_METHOD

BEGIN_METHOD (Object_set, GB_STRING field; GB_VARIANT value)

  const char *key = STRING (field);
  const dwg_ent_generic *obj = (dwg_ent_generic *)THIS->_obj;
  const char *dxfname = THIS->obj->dxfname;
  GB_VALUE *value = (GB_VALUE *)ARG(value);
  const Dwg_Data *dwg = THIS->dwg;
  CDwg_Variant out;

  if (!gb_to_dynapi_value (dwg, value, &out))
    {
      GB.Error (GB_ERR_TYPE);
      return;
    }
  if (!dwg_dynapi_entity_set_value (dwg, dxfname, key, &out, true))
    GB.Error (GB_ERR_TYPE);

END_METHOD

BEGIN_METHOD_VOID (Object_nextfield)

  const dwg_ent_generic *obj = (dwg_ent_generic *)THIS->_obj;
  const char *name = THIS->obj->name;
  CDwg_Variant value;
  Dwg_DYNAPI_field *fields = dwg_dynapi_entity_fields (name);
  Dwg_DYNAPI_field f;
  unsigned n = 0;
  for (; fields && fields->name; fields++) {
    n++;
    if (THIS->iter == n) {
      if (!dwg_dynapi_entity_value (obj, name, fields->name, &value, &f))
        {
          GB.Error (GB_ERR_BOUND);
          return;
        }
      THIS->iter++;
      dynapi_to_gb_value (THIS->dwg, &f, &value);
      return;
    }
  }

  fields = THIS->obj->supertype == DWG_SUPERTYPE_ENTITY
    ? dwg_dynapi_common_entity_fields ()
    : dwg_dynapi_common_object_fields ();
  for (; fields && fields->name; fields++) {
    n++;
    if (THIS->iter == n) {
      if (!dwg_dynapi_entity_value (obj, name, fields->name, &value, &f))
        {
          GB.Error (GB_ERR_BOUND);
          return;
        }
      THIS->iter++;
      dynapi_to_gb_value (THIS->dwg, &f, &value);
      return;
    }
  }
  GB.StopEnum();

END_METHOD

GB_DESC DwgObject_Desc[] =
  {
    GB_DECLARE ("DwgObject", sizeof(CDwgObject)), GB_VIRTUAL_CLASS (),
    GB_PROPERTY_READ ("Count", "i", Object_fieldcount),
    GB_METHOD ("Copy", "o", Object_Copy, NULL),
    GB_METHOD ("Delete", 0, Object_Delete, NULL),
    GB_METHOD ("GetExtensionDictionary", "_Dictionary;",
               Object_GetExtensionDictionary, NULL),
    /* get/set Object fields by fieldname */
    GB_METHOD ("_get", "v", Object_get, "(Field)s"),
    GB_METHOD ("_put", NULL,Object_set, "(Value)v(Field)s"),
    GB_METHOD ("_next", "s",Object_nextfield, NULL),
    GB_END_DECLARE
  };

GB_DESC DwgEntity_Desc[] =
  {
    GB_DECLARE ("DwgEntity", sizeof(CDwgEntity)), GB_VIRTUAL_CLASS (),
    GB_PROPERTY_READ ("Count", "i", Object_fieldcount),
    GB_METHOD ("Copy", "o", Object_Copy, NULL),
    GB_METHOD ("Delete", 0, Object_Delete, NULL),
    GB_METHOD ("GetExtensionDictionary", "_Dictionary;",
               Object_GetExtensionDictionary, NULL),
    /* get/set Object fields by fieldname */
    GB_METHOD ("_get", "v", Object_get, "(Field)s"),
    GB_METHOD ("_put", NULL,Object_set, "(Value)v(Field)s"),
    GB_METHOD ("_next", "s",Object_nextfield, NULL),
    GB_END_DECLARE
  };

#define DWG_OBJECT(token)                                \
GB_DESC token##_Desc[] =                                 \
  {                                                      \
    GB_DECLARE (#token, sizeof(C##token)),               \
    GB_INHERITS ("DwgObject"),                           \
    GB_END_DECLARE                                       \
  };
#define DWG_OBJECT2(obj,token)                           \
GB_DESC obj##_Desc[] =                                   \
  {                                                      \
    GB_DECLARE (#token, sizeof(C##obj)),                 \
    GB_INHERITS ("DwgObject"),                           \
    GB_END_DECLARE                                       \
  };

// FIXME: _3D prefix -> 3D class. strip _
#define COMMON_ENTITY_PRE(obj,token,gbname)              \
GB_DESC obj##_Desc[] =                                   \
  {                                                      \
    GB_DECLARE (gbname, sizeof(C##obj)),                 \
    GB_INHERITS ("DwgEntity")

#define COMMON_ENTITY_POST                               \
    GB_END_DECLARE                                       \
  }

/* Object Specific methods: */
#define DWG_ENTITY_NONE

// INSERT, MINSERT (BlockRef)
#define DWG_ENTITY_INSERT                                           \
  GB_METHOD ("ConvertToAnonymousBlock", 0, Insert_ConvertToAnonymousBlock, NULL), \
  GB_METHOD ("GetAttributes", "o", Insert_GetAttributes, NULL),     \
  GB_METHOD ("ConvertToStaticBlock", 0, DynBlock_ConvertToStaticBlock, "(name)s"), \
  GB_METHOD ("GetDynamicBlockProperties", "o", DynBlock_GetDynamicBlockProperties, NULL),

// POLYLINE_2D, LWPOLYLINE, ...
#define DWG_ENTITY_PLINE                                            \
  GB_METHOD ("AppendVertex", 0, PLine_AppendVertex, "(point)f[3]"), \
  GB_METHOD ("GetBulge", "f", PLine_GetBulge, "(index)i"),
#define DWG_ENTITY_PMESH                                            \
  GB_METHOD ("AppendVertex", 0, PLine_AppendVertex, "(point)f[3]"),
#define DWG_ENTITY__3DFACE                                          \
  GB_METHOD ("GetInvisibleEdge", "b", _3DFace_GetInvisibleEdge, "(index)i"),
#define DWG_ENTITY_SPLINE                                                 \
  GB_METHOD ("DeleteFitPoint", 0, Spline_DeleteFitPoint, "(index)i"),     \
  GB_METHOD ("ElevateOrder", 0, Spline_ElevateOrder, "(order)i"),         \
  GB_METHOD ("GetControlPoint", "o", Spline_GetControlPoint, "(index)i"), \
  GB_METHOD ("GetFitPoint", "o", Spline_GetFitPoint, "(index)i"),

#define DWG_ENTITY(token)                                \
  COMMON_ENTITY_PRE (token, token, #token),              \
  COMMON_ENTITY_POST;
#define DWG_ENTITY2(obj, token)                          \
  COMMON_ENTITY_PRE (obj, token, #token),                \
  COMMON_ENTITY_POST;
#define DWG_ENTITY4_EXTRA(obj, token, gbname, extra)     \
  COMMON_ENTITY_PRE (obj, token, gbname),                \
  DWG_ENTITY_##extra                                     \
  COMMON_ENTITY_POST;

#include "names.inc"
#undef DWG_OBJECT
#undef DWG_ENTITY

/* Done and missing object specific methods:

/3DFACE.GetInvisibleEdge
3DSOLID.Boolean
REGION.Boolean
BLOCK.Bind
BLOCK.Detach
GROUP.AppendItems
HATCH.AppendInnerLoop
HATCH.AppendOuterLoop
HATCH.Evaluate
IMAGE.ClipBoundary
LAYOUT.GetCanonicalMediaNames
LAYOUT.GetCustomScale
PLOTSETTINGS.GetCanonicalMediaNames
Layers.GenerateUsageData
LEADER.Evaluate
MULTILEADER.GetBlockAttributeValue
MULTILEADER.GetBlockAttributeValue32
/LWPOLYLINE.GetBulge
/POLYLINE_2D.GetBulge
/POLYLINE_{3D,2D,MESH}.AppendVertex
SECTION.GenerateSectionGeometry
SORTENTSTABLE.Block
SORTENTSTABLE.GetFullDrawOrder
SPLINE.DeleteFitPoint
SPLINE.ElevateOrder
SPLINE.GetControlPoint
SPLINE.GetFitPoint
STYLE.GetFont
TABLE.ClearTableStyleOverrides
TABLE.CreateContent
TABLE.DeleteCellContent
TABLE.DeleteColumns
TABLE.DeleteContent
TABLE.DeleteRows
TABLE.EnableMergeAll
TABLE.FormatValue
TABLE.GenerateLayout
TABLE.GetAlignment
TABLE.GetAlignment2
TABLE.GetAttachmentPoint
TABLE.GetAutoScale
TABLE.GetAutoScale2
TABLE.GetBackgroundColor
TABLE.GetBackgroundColorNone
TABLE.GetBlockAttributeValue
TABLE.GetBlockAttributeValue2
TABLE.GetBlockAttributeValue32
TABLE.GetBlockRotation
TABLE.GetBlockScale
TABLE.GetBlockTableRecordId
TABLE.GetBlockTableRecordId2
TABLE.GetBlockTableRecordId32
TABLE.GetBreakHeight
TABLE.GetCellAlignment
...
TABLE_STYLE.CreateCellStyle
TABLE_STYLE.CreateCellStyleFromStyle
TABLE_STYLE.DeleteCellStyle
TABLE_STYLE.EnableMergeAll
TABLE_STYLE.GetAlignment
TABLE_STYLE.GetAlignment2
TABLE_STYLE.GetBackgroundColor
TABLE_STYLE.GetBackgroundColorNone
TABLE_STYLE.GetDataType
TABLE_STYLE.GetDataType2
TABLE_STYLE.GetFormat
TABLE_STYLE.GetFormat2
TEXT.FieldCode
MTEXT.FieldCode
UNDERLAY.ClipBoundary
VIEWPORT.Display
ViewPorts.DeleteConfiguration
PLOT.DisplayPlotPreview

*/
