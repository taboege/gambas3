/***************************************************************************

  c_ents.c

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

#define _C_ENTS_C

#include "c_dwg.h"
#include "c_ents.h"

#define THIS ((CModelSpace *)_object)

// List of ModelSpace, PaperSpace or Blocks entities.
// Returns dwg_ent_generic Object
BEGIN_METHOD(Entities_get, GB_INTEGER index;)
  unsigned index = (unsigned)VARG(index);
  if (index >= THIS->blkhdr->num_owned) {
    GB.Error(GB_ERR_BOUND);
    GB.ReturnVariant (NULL);
    return;
  }
  GB.ReturnObject (handle_to_gb (THIS->dwg, THIS->blkhdr->entities[index]));
END_METHOD

// Returns dwg_ent_generic Object
BEGIN_METHOD_VOID(Entities_next)
  if (THIS->iter >= THIS->blkhdr->num_owned)
    GB.StopEnum();
  else
    GB.ReturnObject (handle_to_gb (THIS->dwg, THIS->blkhdr->entities[THIS->iter++]));
END_METHOD
  
BEGIN_PROPERTY(Entities_Count)
  GB.ReturnInteger (THIS->blkhdr->num_owned);
END_PROPERTY

BEGIN_METHOD(Blk_Add3DFace, GB_OBJECT pt1; GB_OBJECT pt2; GB_OBJECT pt3; GB_OBJECT pt4)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity__3DFACE *_obj;
  dwg_point_3d pt1, pt2, pt3, pt4;
  SET_PT1 (pt1);
  SET_PT1 (pt2);
  SET_PT1 (pt3);
  SET_PT1 (pt4);
  _obj = dwg_add_3DFACE (blkhdr, &pt1, &pt2, &pt3, &pt4);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

BEGIN_METHOD(Blk_Add3DMesh, GB_INTEGER m; GB_INTEGER n; GB_OBJECT matrix)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_POLYLINE_MESH *_obj;
  unsigned m = VARG(m);
  unsigned n = VARG(n);
  const unsigned num = m * n;
  GB_ARRAY matrix = (GB_ARRAY)VARG(matrix);
  dwg_point_3d *verts = calloc (num, sizeof (dwg_point_3d));
  if (!verts) {
    GB.Error(GB_ERR_BOUND);
    GB.ReturnVariant (NULL);
    return;
  }
  if (GB.Array.Count(matrix) / 3 != num) {
    GB.Error("Invalid number of 3DPoints &1", "matrix");
    GB.ReturnVariant (NULL);
    return;
  }
  for (unsigned i = 0, j = 0; i < num; i++) {
    verts[i].x = *(double*)GB.Array.Get (matrix, j++);
    verts[i].y = *(double*)GB.Array.Get (matrix, j++);
    verts[i].z = *(double*)GB.Array.Get (matrix, j++);
  }
  _obj = dwg_add_POLYLINE_MESH (blkhdr, m, n, verts);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

BEGIN_METHOD(Blk_Add3DPoly, GB_OBJECT points)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_POLYLINE_3D *_obj;
  GB_ARRAY *ptsarr = (GB_ARRAY)VARG (points);
  unsigned num_pts = GB.Array.Count (ptsarr) / 3;
  dwg_point_3d *pts;
  if (GB.Array.Count(ptsarr) % 3) {
    GB.Error("Not array of 3DPoints &1", "points");
    GB.ReturnVariant (NULL);
    return;
  }
  pts = calloc (num_pts, sizeof (dwg_point_3d));
  for (unsigned i = 0, j = 0; i < num_pts; i++) {
    pts[i].x = *(double*)GB.Array.Get (ptsarr, j++);
    pts[i].y = *(double*)GB.Array.Get (ptsarr, j++);
    pts[i].z = *(double*)GB.Array.Get (ptsarr, j++);
  }
  _obj = dwg_add_POLYLINE_3D (blkhdr, num_pts, pts);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (Center)f[3](Radius)f(StartAngle)f(EndAngle)f
BEGIN_METHOD(Blk_AddArc, GB_OBJECT center; GB_FLOAT radius; GB_FLOAT start_angle; GB_FLOAT end_angle)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_ARC *_obj;
  double radius = (double)VARG(radius);
  double start_angle = (double)VARG(start_angle);
  double end_angle = (double)VARG(end_angle);
  dwg_point_3d center;
  SET_PT1 (center);
  _obj = dwg_add_ARC (blkhdr, &center, radius, start_angle, end_angle);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (Center)f[3](Radius)f
BEGIN_METHOD(Blk_AddCircle, GB_OBJECT center; GB_FLOAT radius)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_CIRCLE *_obj;
  double radius = (double)VARG(radius);
  dwg_point_3d center;
  SET_PT1 (center);
  _obj = dwg_add_CIRCLE (blkhdr, &center, radius);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (AngleVertex)f[3](FirstEndPoint)f[3](SecondEndPoint)f[3](TextPoint)f[3]
BEGIN_METHOD(Blk_AddDim3PointAngular, GB_OBJECT center; GB_OBJECT xline1_pt; GB_OBJECT xline2_pt;
                                      GB_OBJECT text_midpt)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_DIMENSION_ANG3PT *_obj;
  dwg_point_3d center, xline1_pt, xline2_pt, text_midpt;
  SET_PT1 (center);
  SET_PT1 (xline1_pt);
  SET_PT1 (xline2_pt);
  SET_PT1 (text_midpt);
  _obj = dwg_add_DIMENSION_ANG3PT (blkhdr, &center, &xline1_pt, &xline2_pt, &text_midpt);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

BEGIN_METHOD(Blk_AddDimAligned, GB_OBJECT xline1_pt; GB_OBJECT xline2_pt;
                                GB_OBJECT text_midpt)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_DIMENSION_ALIGNED *_obj;
  dwg_point_3d xline1_pt, xline2_pt, text_midpt;
  SET_PT1 (xline1_pt);
  SET_PT1 (xline2_pt);
  SET_PT1 (text_midpt);
  _obj = dwg_add_DIMENSION_ALIGNED (blkhdr, &xline1_pt, &xline2_pt, &text_midpt);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

BEGIN_METHOD(Blk_AddDimAngular, GB_OBJECT center_pt; GB_OBJECT xline1end_pt;
                                GB_OBJECT xline2end_pt; GB_OBJECT text_midpt)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_DIMENSION_ANG2LN *_obj;
  dwg_point_3d center_pt, xline1end_pt, xline2end_pt, text_midpt;
  SET_PT1 (center_pt);
  SET_PT1 (xline1end_pt);
  SET_PT1 (xline2end_pt);
  SET_PT1 (text_midpt);
  _obj = dwg_add_DIMENSION_ANG2LN (blkhdr, &center_pt, &xline1end_pt, &xline2end_pt, &text_midpt);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

BEGIN_METHOD(Blk_AddDimArc, GB_OBJECT center; GB_OBJECT xline1_pt; GB_OBJECT xline2_pt;
                            GB_OBJECT arc_pt)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_ARC_DIMENSION *_obj;
  dwg_point_3d center, xline1_pt, xline2_pt, arc_pt;
  SET_PT1 (center);
  SET_PT1 (xline1_pt);
  SET_PT1 (xline2_pt);
  SET_PT1 (arc_pt);
#ifdef HAVE_DWG_ADD_ARC_DIMENSION
  _obj = dwg_add_ARC_DIMENSION (blkhdr, &center, &xline1_pt, &xline2_pt, &arc_pt);
  GB.ReturnObject (obj_generic_to_gb (_obj));
#else
  GB.Error("Not yet implemented");
  GB.ReturnVariant (NULL);
#endif
END_METHOD

BEGIN_METHOD(Blk_AddDimDiametric, GB_OBJECT chord_pt; GB_OBJECT far_chord_pt;
                                  GB_FLOAT leader_len)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_DIMENSION_DIAMETER *_obj;
  double leader_len = (double)VARG(leader_len);
  dwg_point_3d chord_pt, far_chord_pt;
  SET_PT1 (chord_pt);
  SET_PT1 (far_chord_pt);
  _obj = dwg_add_DIMENSION_DIAMETER (blkhdr, &chord_pt, &far_chord_pt, leader_len);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

BEGIN_METHOD(Blk_AddDimOrdinate, GB_OBJECT feature_location_pt; GB_OBJECT leader_endpt;
                                 GB_BOOLEAN use_x_axis)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_DIMENSION_ORDINATE *_obj;
  bool use_x_axis = (bool)VARG(use_x_axis);
  dwg_point_3d feature_location_pt, leader_endpt;
  SET_PT1 (feature_location_pt);
  SET_PT1 (leader_endpt);
  _obj = dwg_add_DIMENSION_ORDINATE (blkhdr, &feature_location_pt, &leader_endpt, use_x_axis);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (Center)f[3](ChordPoint)f[3](LeaderLength)f
BEGIN_METHOD(Blk_AddDimRadial, GB_OBJECT center_pt; GB_OBJECT chord_pt;
                               GB_FLOAT leader_len)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_DIMENSION_RADIUS *_obj;
  double leader_len = (double)VARG(leader_len);
  dwg_point_3d center_pt, chord_pt;
  SET_PT1 (center_pt);
  SET_PT1 (chord_pt);
  _obj = dwg_add_DIMENSION_RADIUS (blkhdr, &center_pt, &chord_pt, leader_len);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (Center)f[3](ChordPoint)f[3](LeaderLength)f
BEGIN_METHOD(Blk_AddDimRadialLarge, GB_OBJECT center_pt; GB_OBJECT first_arc_pt;
                                    GB_OBJECT ovr_center_pt; GB_OBJECT jog_pt;
                                    GB_FLOAT jog_angle)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_LARGE_RADIAL_DIMENSION *_obj;
  dwg_point_3d center_pt, first_arc_pt, ovr_center_pt, jog_pt;
  double jog_angle = (double)VARG(jog_angle);
  SET_PT1 (center_pt);
  SET_PT1 (first_arc_pt);
  SET_PT1 (ovr_center_pt);
  SET_PT1 (jog_pt);
  _obj = dwg_add_LARGE_RADIAL_DIMENSION (blkhdr, &center_pt, &first_arc_pt, &ovr_center_pt,
                                         &jog_pt, jog_angle);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

BEGIN_METHOD(Blk_AddDimRotated, GB_OBJECT xline1_pt; GB_OBJECT xline2_pt;
                                GB_OBJECT def_pt; GB_FLOAT rotation_angle)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_DIMENSION_LINEAR *_obj;
  dwg_point_3d xline1_pt, xline2_pt, def_pt;
  double rotation_angle = (double)VARG(rotation_angle);
  SET_PT1 (xline1_pt);
  SET_PT1 (xline2_pt);
  SET_PT1 (def_pt);
  _obj = dwg_add_DIMENSION_LINEAR (blkhdr, &xline1_pt, &xline2_pt, &def_pt, rotation_angle);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (Center)f[3](MajorAxis)f(RadiusRatio)f
BEGIN_METHOD(Blk_AddEllipse, GB_OBJECT center; GB_FLOAT major_axis; GB_FLOAT radius_ratio)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_ELLIPSE *_obj;
  double major_axis = (double)VARG(major_axis);
  double radius_ratio = (double)VARG(radius_ratio);
  dwg_point_3d center;
  SET_PT1 (center);
  _obj = dwg_add_ELLIPSE (blkhdr, &center, major_axis, radius_ratio);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (PatternType)i(PatternName)s(Associativity)b(PathObjects)o[]
BEGIN_METHOD(Blk_AddHatch, GB_INTEGER pattern_type; GB_STRING name; GB_BOOLEAN is_associative;
                           GB_OBJECT pathobjs)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_HATCH *_obj;
  int pattern_type = (int)VARG(pattern_type);
  char *name = STRING(name);
  bool is_associative = (bool)VARG(is_associative);
  GB_ARRAY paths = (GB_ARRAY)VARG(pathobjs);
  int num = GB.Array.Count(paths);
  const Dwg_Object **pathobjs = calloc (num, sizeof (Dwg_Object*));
  for (unsigned i = 0; i < num; i++) {
    pathobjs[i] = *(Dwg_Object**)GB.Array.Get (paths, i);
  }
  _obj = dwg_add_HATCH (blkhdr, pattern_type, name, is_associative,
                        num, pathobjs);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (Points)f[](Annotation)o(Type)i
BEGIN_METHOD(Blk_AddLeader, GB_OBJECT points; GB_OBJECT annotation; GB_INTEGER type)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_LEADER *_obj;
  GB_ARRAY points = (GB_ARRAY)VARG(points);
  Dwg_Entity_MTEXT* annotation;
  CDwgObject *annot = (CDwgObject *)VARG(annotation);
  int type = (int)VARG(type);
  int num = GB.Array.Count(points) / 3;
  int error;
  dwg_point_3d *pts;
  Dwg_Object *ann = annot ? dwg_ent_generic_to_object (annot->obj, &error) : NULL;

  if (ann && ann->fixedtype != DWG_TYPE_MTEXT) {
    GB.Error("Not an MTEXT &1", "annotation");
    GB.ReturnVariant (NULL);
    return;
  }
  if (GB.Array.Count(points) % 3) { // needs to be dividable by 3
    GB.Error("Not array of 3DPoints &1", "points");
    GB.ReturnVariant (NULL);
    return;
  }
  pts = calloc (num, sizeof (dwg_point_3d));
  for (unsigned i = 0, j = 0; i < num; i++) {
    pts[i].x = *(double*)GB.Array.Get (points, j++);
    pts[i].y = *(double*)GB.Array.Get (points, j++);
    pts[i].z = *(double*)GB.Array.Get (points, j++);
  }
  _obj = dwg_add_LEADER (blkhdr, num, pts, annotation, type);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (Points2D)f[]
BEGIN_METHOD(Blk_AddLightWeightPolyline, GB_OBJECT points2d)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_LWPOLYLINE *_obj;
  GB_ARRAY points = (GB_ARRAY)VARG(points2d);
  int num = GB.Array.Count(points) / 2;
  dwg_point_2d *pts;

  if (GB.Array.Count(points) % 2) { // needs to be dividable by 2
    GB.Error("Not array of 2DPoints &1", "points2d");
    GB.ReturnVariant (NULL);
    return;
  }
  pts = calloc (num, sizeof (dwg_point_2d));
  for (unsigned i = 0, j = 0; i < num; i++) {
    pts[i].x = *(double*)GB.Array.Get (points, j++);
    pts[i].y = *(double*)GB.Array.Get (points, j++);
  }
  _obj = dwg_add_LWPOLYLINE (blkhdr, num, pts);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

BEGIN_METHOD(Blk_AddLine, GB_OBJECT pt1; GB_OBJECT pt2)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_LINE *_obj;
  dwg_point_3d pt1, pt2;
  SET_PT1 (pt1);
  SET_PT1 (pt2);
  _obj = dwg_add_LINE (blkhdr, &pt1, &pt2);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// InsPoint)f[3](Name)s(XScale)f(YScale)f(ZScale)f(Rotation)f(NumRows)i(NumColumns)i(RowSpacing)f(ColumnsSpacing)f
BEGIN_METHOD(Blk_AddMInsertBlock, GB_OBJECT ins_pt; GB_STRING name;
             GB_FLOAT xscale; GB_FLOAT yscale; GB_FLOAT zscale; GB_FLOAT rotation;
             GB_INTEGER num_rows; GB_INTEGER num_cols;
             GB_FLOAT row_spacing; GB_FLOAT col_spacing)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_MINSERT *_obj;
  dwg_point_3d ins_pt;
  char *name = STRING(name);
  double xscale = (double)VARG(xscale);
  double yscale = (double)VARG(yscale);
  double zscale = (double)VARG(zscale);
  double rotation = (double)VARG(rotation);
  int num_rows = (int)VARG(num_rows);
  int num_cols = (int)VARG(num_cols);
  double row_spacing = (double)VARG(row_spacing);
  double col_spacing = (double)VARG(col_spacing);
  if (!dwg_find_tablehandle (THIS->dwg, name, "BLOCK")) {
    GB.Error (GB_ERR_TYPE); // block not yet defined
    GB.ReturnVariant (NULL);
    return;
  }
  SET_PT1 (ins_pt);
  _obj = dwg_add_MINSERT (blkhdr, &ins_pt, name, xscale, yscale, zscale, rotation,
                          num_rows, num_cols, row_spacing, col_spacing);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (InsPoint)f[3](Name)s(XScale)f(YScale)f(ZScale)f(Rotation)
BEGIN_METHOD(Blk_AddInsertBlock, GB_OBJECT ins_pt; GB_STRING name;
             GB_FLOAT xscale; GB_FLOAT yscale; GB_FLOAT zscale; GB_FLOAT rotation)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_INSERT *_obj;
  dwg_point_3d ins_pt;
  char *name = STRING(name);
  double xscale = (double)VARG(xscale);
  double yscale = (double)VARG(yscale);
  double zscale = (double)VARG(zscale);
  double rotation = (double)VARG(rotation);
  if (!dwg_find_tablehandle (THIS->dwg, name, "BLOCK")) {
    GB.Error (GB_ERR_TYPE); // block not yet defined
    GB.ReturnVariant (NULL);
    return;
  }
  SET_PT1 (ins_pt);
  _obj = dwg_add_INSERT (blkhdr, &ins_pt, name, xscale, yscale, zscale, rotation);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (Points)f[](LeaderLineIndex)i
BEGIN_METHOD(Blk_AddMLeader, GB_OBJECT points; GB_INTEGER leaderline_index)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_MULTILEADER *_obj;
  GB_ARRAY points = (GB_ARRAY)VARG(points);
  int leaderline_index = (int)VARG(leaderline_index);
  int num = GB.Array.Count(points) / 3;
  dwg_point_3d *pts;

  if (GB.Array.Count(points) % 3) { // needs to be dividable by 3
    GB.Error("Not array of 3DPoints &1", "points");
    GB.ReturnVariant (NULL);
    return;
  }
  pts = calloc (num, sizeof (dwg_point_3d));
  for (unsigned i = 0, j = 0; i < num; i++) {
    pts[i].x = *(double*)GB.Array.Get (points, j++);
    pts[i].y = *(double*)GB.Array.Get (points, j++);
    pts[i].z = *(double*)GB.Array.Get (points, j++);
  }
#ifdef HAVE_DWG_ADD_MULTILEADER 
  _obj = dwg_add_MULTILEADER (blkhdr, num, pts, leaderline_index);
  GB.ReturnObject (obj_generic_to_gb (_obj));
#else
  GB.Error("Not yet implemented");
  GB.ReturnVariant (NULL);
#endif
END_METHOD

// (Points3D)f[]
BEGIN_METHOD(Blk_AddMLine, GB_OBJECT points3d)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_MLINE *_obj;
  GB_ARRAY points = (GB_ARRAY)VARG(points3d);
  int num = GB.Array.Count(points) / 3;
  dwg_point_3d *pts;

  if (GB.Array.Count(points) % 3) { // needs to be dividable by 3
    GB.Error("Not array of 3DPoints &1", "points3d");
    GB.ReturnVariant (NULL);
    return;
  }
  pts = calloc (num, sizeof (dwg_point_3d));
  for (unsigned i = 0, j = 0; i < num; i++) {
    pts[i].x = *(double*)GB.Array.Get (points, j++);
    pts[i].y = *(double*)GB.Array.Get (points, j++);
    pts[i].z = *(double*)GB.Array.Get (points, j++);
  }
  _obj = dwg_add_MLINE (blkhdr, num, pts);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (InsPoint)f[3](Width)f(Text)s
BEGIN_METHOD(Blk_AddMText, GB_OBJECT ins_pt; GB_FLOAT width; GB_STRING text)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_MTEXT *_obj;
  dwg_point_3d ins_pt;
  double width = (double)VARG(width);
  char *text = STRING(text);
  _obj = dwg_add_MTEXT (blkhdr, &ins_pt, width, text);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (Point)f[3]
BEGIN_METHOD(Blk_AddPoint, GB_OBJECT point)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_POINT *_obj;
  dwg_point_3d point;
  SET_PT1 (point);
  _obj = dwg_add_POINT (blkhdr, &point);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

BEGIN_METHOD(Blk_AddPolyfaceMesh, GB_OBJECT verts; GB_OBJECT faces)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_POLYLINE_PFACE *_obj;
  GB_ARRAY vertarr = (GB_ARRAY)VARG(verts);
  unsigned numverts = GB.Array.Count(vertarr) / 3;
  GB_ARRAY facearr = (GB_ARRAY)VARG(faces);
  unsigned numfaces = GB.Array.Count(facearr) / 4;
  dwg_point_3d *verts = calloc (numverts, sizeof (dwg_point_3d));
  dwg_face *faces = calloc (numfaces, sizeof (dwg_face));
  if (GB.Array.Count(verts) % 3) { // needs to be dividable by 3
    GB.Error("Not array of 3DPoints &1", "points3d");
    GB.ReturnVariant (NULL);
    return;
  }
  if (GB.Array.Count(faces) % 4) { // needs to be dividable by 4
    GB.Error(GB_ERR_BOUND);
    GB.ReturnVariant (NULL);
    return;
  }
  for (unsigned i = 0, j = 0; i < numverts; i++) {
    verts[i].x = *(double*)GB.Array.Get (vertarr, j++);
    verts[i].y = *(double*)GB.Array.Get (vertarr, j++);
    verts[i].z = *(double*)GB.Array.Get (vertarr, j++);
  }
  for (unsigned i = 0, j = 0; i < numfaces; i++) {
    faces[i][0] = *(short*)GB.Array.Get (facearr, j++);
    faces[i][1] = *(short*)GB.Array.Get (facearr, j++);
    faces[i][2] = *(short*)GB.Array.Get (facearr, j++);
    faces[i][3] = *(short*)GB.Array.Get (facearr, j++);
  }
_obj = dwg_add_POLYLINE_PFACE (blkhdr, numverts, numfaces, verts, faces);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (Points3D)f[]
BEGIN_METHOD(Blk_AddPolyline, GB_OBJECT points3d)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_POLYLINE_2D *_obj;
  GB_ARRAY points = (GB_ARRAY)VARG(points3d);
  int num = GB.Array.Count(points) / 3;
  dwg_point_2d *pts;

  if (GB.Array.Count(points) % 3) { // needs to be dividable by 3, with z ignored
    GB.Error("Not array of 3DPoints &1", "points3d");
    GB.ReturnVariant (NULL);
    return;
  }
  pts = calloc (num, sizeof (dwg_point_2d));
  for (unsigned i = 0, j = 0; i < num; i++) {
    pts[i].x = *(double*)GB.Array.Get (points, j++);
    pts[i].y = *(double*)GB.Array.Get (points, j++);
    j++; // ignore each z
  }
  _obj = dwg_add_POLYLINE_2D (blkhdr, num, pts);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (Center)f[3](Width)f(Height)f
BEGIN_METHOD(Blk_AddPViewport, GB_STRING name; GB_OBJECT center; GB_FLOAT width; GB_FLOAT height)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr; // FIXME: must be PaperSpace
  Dwg_Entity_VIEWPORT *_obj;
  char *name = STRING(name);
  double width = (double)VARG(width);
  double height = (double)VARG(height);
  dwg_point_3d center;
  SET_PT1 (center);
  _obj = dwg_add_VIEWPORT (blkhdr, name /*, &center, width, height*/);
  memcpy (&_obj->center, &center, sizeof (dwg_point_3d));
  _obj->width = width;
  _obj->height = height;
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// Note: VBA names it just AddRaster.
// (FileName)s(InsPt)f[3](Scalefactor)f(RotationAngle)f
BEGIN_METHOD(Blk_AddRasterImage, GB_STRING file_path; GB_OBJECT ins_pt;
                                 GB_FLOAT scale_factor; GB_FLOAT rotation_angle)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_IMAGE *_obj;
  char *file_path = STRING(file_path);
  double scale_factor = (double)VARG(scale_factor);
  double rotation_angle = (double)VARG(rotation_angle);
  dwg_point_3d ins_pt;
  SET_PT1 (ins_pt);
  _obj = dwg_add_IMAGE (blkhdr, file_path, &ins_pt, scale_factor, rotation_angle);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (Point1)f[3](Point2)f[3]
BEGIN_METHOD(Blk_AddRay, GB_OBJECT pt1; GB_OBJECT pt2)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_RAY *_obj;
  dwg_point_3d pt1, pt2;
  SET_PT1 (pt1);
  SET_PT1 (pt2);
  _obj = dwg_add_RAY (blkhdr, &pt1, &pt2);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (Name)s(ins_pt)f[3](scale_factor)f(oblique)f
BEGIN_METHOD(Blk_AddShape, GB_STRING name; GB_OBJECT ins_pt;
                           GB_FLOAT scale_factor; GB_FLOAT oblique_angle)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_SHAPE *_obj;
  char *name = STRING(name);
  double scale_factor = (double)VARG(scale_factor);
  double oblique_angle = (double)VARG(oblique_angle);
  dwg_point_3d ins_pt;
  SET_PT1 (ins_pt);
  _obj = dwg_add_SHAPE (blkhdr, name, &ins_pt, scale_factor, oblique_angle);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (pt1)f[3](pt2)f[3](pt3)f[3](pt4)f[3]
BEGIN_METHOD(Blk_AddSolid, GB_OBJECT pt1; GB_OBJECT pt2; GB_OBJECT pt3; GB_OBJECT pt4)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_SOLID *_obj;
  dwg_point_3d pt1;
  dwg_point_2d pt2, pt3, pt4;
  SET_PT1 (pt1);
  SET_PT2D (pt2);
  SET_PT2D (pt3);
  SET_PT2D (pt4);
  _obj = dwg_add_SOLID (blkhdr, &pt1, &pt2, &pt3, &pt4);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (fit_pts)f[](beg_tan_vec)f[3](end_tan_vec)f[3]
BEGIN_METHOD(Blk_AddSpline, GB_OBJECT fit_pts; GB_OBJECT beg_tan_vec; GB_OBJECT end_tan_vec)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_SPLINE *_obj;
  GB_ARRAY fitpts_arr = (GB_ARRAY)VARG(fit_pts);
  int num = GB.Array.Count(fitpts_arr) / 3;
  dwg_point_3d *pts, beg_tan_vec, end_tan_vec;

  if (GB.Array.Count(fitpts_arr) % 3) { // needs to be dividable by 3
    GB.Error("Not array of 3DPoints &1", "fit_pts");
    GB.ReturnVariant (NULL);
    return;
  }
  SET_PT1 (beg_tan_vec);
  SET_PT1 (end_tan_vec);
  pts = calloc (num, sizeof (dwg_point_3d));
  for (unsigned i = 0, j = 0; i < num; i++) {
    pts[i].x = *(double*)GB.Array.Get (fitpts_arr, j++);
    pts[i].y = *(double*)GB.Array.Get (fitpts_arr, j++);
    pts[i].z = *(double*)GB.Array.Get (fitpts_arr, j++);
  }
  _obj = dwg_add_SPLINE (blkhdr, num, pts, &beg_tan_vec, &end_tan_vec);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (text)s(ins_pt)f[3](height)f
BEGIN_METHOD(Blk_AddText, GB_STRING text; GB_OBJECT ins_pt; GB_FLOAT height)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_TEXT *_obj;
  dwg_point_3d ins_pt;
  double height = (double)VARG(height);
  char *text = STRING(text);
  _obj = dwg_add_TEXT (blkhdr, text, &ins_pt, height);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (text)s(ins_pt)f[3](x_direction)f[3]
BEGIN_METHOD(Blk_AddTolerance, GB_STRING text; GB_OBJECT ins_pt; GB_OBJECT x_direction)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_TOLERANCE *_obj;
  dwg_point_3d ins_pt, x_direction;
  char *text = STRING(text);
  SET_PT1 (ins_pt);
  SET_PT1 (x_direction);
  _obj = dwg_add_TOLERANCE (blkhdr, text, &ins_pt, &x_direction);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (points3d)f[]
BEGIN_METHOD(Blk_AddTrace, GB_OBJECT points3d)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_TRACE *_obj;
  GB_ARRAY ptsarr = (GB_ARRAY)VARG(points3d);
  unsigned j = 0;
  dwg_point_3d pt1;
  dwg_point_2d pt2, pt3, pt4;
  if (GB.Array.Count(ptsarr) != 12) {
    GB.Error(GB_ERR_BOUND);
    GB.ReturnVariant (NULL);
    return;
  }
  pt1.x = *(double*)GB.Array.Get (ptsarr, j++);
  pt1.y = *(double*)GB.Array.Get (ptsarr, j++);
  pt1.z = *(double*)GB.Array.Get (ptsarr, j++);

  pt2.x = *(double*)GB.Array.Get (ptsarr, j++);
  pt2.y = *(double*)GB.Array.Get (ptsarr, j++); j++;
  pt3.x = *(double*)GB.Array.Get (ptsarr, j++);
  pt3.y = *(double*)GB.Array.Get (ptsarr, j++); j++;
  pt4.x = *(double*)GB.Array.Get (ptsarr, j++);
  pt4.y = *(double*)GB.Array.Get (ptsarr, j++); j++;

  _obj = dwg_add_TRACE (blkhdr, &pt1, &pt2, &pt3, &pt4);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (Point1)f[3](Point2)f[3]
BEGIN_METHOD(Blk_AddXLine, GB_OBJECT pt1; GB_OBJECT pt2)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_XLINE *_obj;
  dwg_point_3d pt1, pt2;
  SET_PT1 (pt1);
  SET_PT1 (pt2);
  _obj = dwg_add_RAY (blkhdr, &pt1, &pt2);
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD

// (origin)f[3](length)f(width)f(height)f
BEGIN_METHOD(Blk_AddBox, GB_OBJECT origin; GB_FLOAT length; GB_FLOAT width; GB_FLOAT height)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_3DSOLID *_obj;
  dwg_point_3d origin;
  dwg_point_3d normal = (dwg_point_3d) { 0, 0, 1 };
  double length = (double)VARG(length);
  double width = (double)VARG(width);
  double height = (double)VARG(height);
  // todo check doubles are >0.0
  SET_PT1 (origin);
#ifdef HAVE_DWG_ADD_BOX
  _obj = dwg_add_BOX (blkhdr, &origin, &normal, length, width, height);
  GB.ReturnObject (obj_generic_to_gb (_obj));
#else
  GB.Error("Not yet implemented");
  GB.ReturnVariant (NULL);
#endif
END_METHOD

// (center)f[3](base_radius)f(height)f
BEGIN_METHOD(Blk_AddCone, GB_OBJECT center; GB_FLOAT base_radius; GB_FLOAT height)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_3DSOLID *_obj;
  dwg_point_3d center;
  dwg_point_3d normal = (dwg_point_3d) { 0, 0, 1 };
  double base_radius = (double)VARG(base_radius);
  double height = (double)VARG(height);
  // todo check doubles are >0.0
  SET_PT1 (center);
#ifdef HAVE_DWG_ADD_CONE
  _obj = dwg_add_CONE (blkhdr, &center, &normal, height, base_radius, 0.0, 0.0);
  GB.ReturnObject (obj_generic_to_gb (_obj));
#else
  GB.Error("Not yet implemented");
  GB.ReturnVariant (NULL);
#endif
END_METHOD

BEGIN_METHOD(Blk_AddCylinder, GB_OBJECT center; GB_FLOAT radius; GB_FLOAT height)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_3DSOLID *_obj;
  dwg_point_3d center;
  dwg_point_3d normal = (dwg_point_3d) { 0, 0, 1 };
  double radius = (double)VARG(radius);
  double height = (double)VARG(height);
  // todo check doubles are >0.0
  SET_PT1 (center);
#ifdef HAVE_DWG_ADD_CYLINDER
  _obj = dwg_add_CYLINDER (blkhdr, &center, &normal, height, radius, radius, radius);
  GB.ReturnObject (obj_generic_to_gb (_obj));
#else
  GB.Error("Not yet implemented");
  GB.ReturnVariant (NULL);
#endif
END_METHOD

// (center)f[3](length)f(width)f(height)f
BEGIN_METHOD(Blk_AddWedge, GB_OBJECT center; GB_FLOAT length; GB_FLOAT width; GB_FLOAT height)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_3DSOLID *_obj;
  dwg_point_3d center;
  dwg_point_3d normal = (dwg_point_3d) { 0, 0, 1 };
  double length = (double)VARG(length);
  double width = (double)VARG(width);
  double height = (double)VARG(height);
  // todo check doubles are >0.0
  SET_PT1 (center);
#ifdef HAVE_DWG_ADD_WEDGE
  _obj = dwg_add_WEDGE (blkhdr, &center, &normal, length, width, height);
  GB.ReturnObject (obj_generic_to_gb (_obj));
#else
  GB.Error("Not yet implemented");
  GB.ReturnVariant (NULL);
#endif
END_METHOD

// (Center)f[3](Radius)f
BEGIN_METHOD(Blk_AddSphere, GB_OBJECT center; GB_FLOAT radius)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_3DSOLID *_obj;
  dwg_point_3d center;
  dwg_point_3d normal = (dwg_point_3d) { 0, 0, 1 };
  double radius = (double)VARG(radius);
  // todo check radius is >0.0
  SET_PT1 (center);
#ifdef HAVE_DWG_ADD_SPHERE
  _obj = dwg_add_SPHERE (blkhdr, &center, &normal, radius);
  GB.ReturnObject (obj_generic_to_gb (_obj));
#else
  GB.Error("Not yet implemented");
  GB.ReturnVariant (NULL);
#endif
END_METHOD

// (Center)f[3](TorusRadius)f(TubeRadius)f
BEGIN_METHOD(Blk_AddTorus, GB_OBJECT center; GB_FLOAT torus_radius; GB_FLOAT tube_radius)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_3DSOLID *_obj;
  dwg_point_3d center;
  dwg_point_3d normal = (dwg_point_3d) { 0, 0, 1 };
  double torus_radius = (double)VARG(torus_radius);
  double tube_radius = (double)VARG(tube_radius);
  // todo check doubles are >0.0
  SET_PT1 (center);
#ifdef HAVE_DWG_ADD_TORUS
  _obj = dwg_add_TORUS (blkhdr, &center, &normal, torus_radius, tube_radius);
  GB.ReturnObject (obj_generic_to_gb (_obj));
#else
  GB.Error("Not yet implemented");
  GB.ReturnVariant (NULL);
#endif
END_METHOD

/* Not yet implemented 3DSOLID entities */

// (center)f[3](radius)f(height)f
BEGIN_METHOD(Blk_AddPyramid, GB_OBJECT center; GB_FLOAT radius; GB_FLOAT height)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_3DSOLID *_obj;
  dwg_point_3d center;
  dwg_point_3d normal = (dwg_point_3d) { 0, 0, 1 };
  double height = (double)VARG(height);
  double radius = (double)VARG(radius);
  int sides = 4;
  double top_radius = 0.0;
  // todo check numbers are >0.0, sides >2
  SET_PT1 (center);
#ifdef HAVE_DWG_ADD_PYRAMID
  _obj = dwg_add_PYRAMID (blkhdr, &center, &normal, height, sides, radius, top_radius);
  GB.ReturnObject (obj_generic_to_gb (_obj));
#else
  GB.Error("Not yet implemented");
  GB.ReturnVariant (NULL);
#endif
END_METHOD

// not yet. (center)f[3](base_radius)f(height)f
BEGIN_METHOD(Blk_AddEllipticalCone, GB_OBJECT center; GB_FLOAT major_radius;
                                    GB_FLOAT minor_radius; GB_FLOAT height)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_3DSOLID *_obj;
  dwg_point_3d center;
  dwg_point_3d normal = (dwg_point_3d) { 0, 0, 1 };
  double major_radius = (double)VARG(major_radius);
  double minor_radius = (double)VARG(minor_radius);
  double height = (double)VARG(height);
  // todo check doubles are >0.0
  SET_PT1 (center);
#ifdef HAVE_DWG_ADD_CYLINDER
  _obj = dwg_add_CYLINDER (blkhdr, &center, &normal, height, major_radius, minor_radius, 0.0);
  //_obj = dwg_add_ELLIPTICAL_CONE (blkhdr, &center, &normal, major_radius, minor_radius, height);
  GB.ReturnObject (obj_generic_to_gb (_obj));
#else
  GB.Error("Not yet implemented");
  GB.ReturnVariant (NULL);
#endif
END_METHOD

// not yet. (center)f[3](major_radius)f(minor_radius)f(height)f
BEGIN_METHOD(Blk_AddEllipticalCylinder, GB_OBJECT center; GB_FLOAT major_radius;
                                        GB_FLOAT minor_radius; GB_FLOAT height)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_3DSOLID *_obj;
  dwg_point_3d center;
  dwg_point_3d normal = (dwg_point_3d) { 0, 0, 1 };
  double major_radius = (double)VARG(major_radius);
  double minor_radius = (double)VARG(minor_radius);
  double height = (double)VARG(height);
  SET_PT1 (center);
#ifdef HAVE_DWG_ADD_CYLINDER
  _obj = dwg_add_CYLINDER (blkhdr, &center, &normal, height, major_radius, minor_radius, minor_radius);
  //_obj = dwg_add_ELLIPTICAL_CYLINDER (blkhdr, &center, &normal, major_radius, minor_radius, height);
  GB.ReturnObject (obj_generic_to_gb (_obj));
#else
  GB.Error("Not yet implemented");
  GB.ReturnVariant (NULL);
#endif
END_METHOD

// not yet. (Profile)o(axis_pt)f[3](axis_dir)f[3](angle)f
BEGIN_METHOD(Blk_AddRevolvedSolid, GB_OBJECT profile; GB_OBJECT axis_pt; GB_OBJECT axis_dir;
                                   GB_FLOAT angle)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_3DSOLID *_obj;
  dwg_point_3d axis_pt, axis_dir;
  Dwg_Object *profile = (Dwg_Object *)VARG(profile);
  double angle = (double)VARG(angle);
  SET_PT1 (axis_pt);
  SET_PT1 (axis_dir);
#ifdef HAVE_DWG_ADD_REVOLVED_SOLID
  _obj = dwg_add_REVOLVED_SOLID (blkhdr, &profile, &axis_pt, &axis_dir, angle);
  GB.ReturnObject (obj_generic_to_gb (_obj));
#else
  GB.Error("Not yet implemented");
  GB.ReturnVariant (NULL);
#endif
END_METHOD

// not yet. (Profile)o(Height)f(TaperAngle)f
BEGIN_METHOD(Blk_AddExtrudedSolid, GB_OBJECT profile; GB_FLOAT height; GB_FLOAT taper_angle)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_3DSOLID *_obj;
  Dwg_Object *profile = (Dwg_Object *)VARG(profile);
  double height = (double)VARG(height);
  double taper_angle = (double)VARG(taper_angle);
  // todo check height is >0.0
#ifdef HAVE_DWG_ADD_EXTRUDED_SOLID
  _obj = dwg_add_EXTRUDED_SOLID (blkhdr, &profile, height, angle);
  GB.ReturnObject (obj_generic_to_gb (_obj));
#else
  GB.Error("Not yet implemented");
  GB.ReturnVariant (NULL);
#endif
END_METHOD

// not yet. (Profile)o(PathObj)o
BEGIN_METHOD(Blk_AddExtrudedSolidAlongPath, GB_OBJECT profile; GB_OBJECT path_obj)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_3DSOLID *_obj;
  Dwg_Object *profile = (Dwg_Object *)VARG(profile);
  Dwg_Object *path_obj = (Dwg_Object *)VARG(path_obj);
#ifdef HAVE_DWG_ADD_EXTRUDED_PATH
  _obj = dwg_add_EXTRUDED_PATH (blkhdr, &profile, &path_obj);
  GB.ReturnObject (obj_generic_to_gb (_obj));
#else
  GB.Error("Not yet implemented");
  GB.ReturnVariant (NULL);
#endif
END_METHOD

// (path_name)s(name)s(ins_pt)f[3](xscale)f(yscale)f(zscale)f(rotation)f(is_overlay)b
BEGIN_METHOD(Blk_AttachExternalReference, GB_STRING path_name; GB_STRING name;
             GB_OBJECT ins_pt; GB_FLOAT xscale; GB_FLOAT yscale; GB_FLOAT zscale;
             GB_FLOAT rotation; GB_BOOLEAN is_overlay)
  Dwg_Object_BLOCK_HEADER *blkhdr = THIS->blkhdr;
  Dwg_Entity_INSERT *_obj;
  dwg_point_3d ins_pt;
  char *path_name = STRING(path_name);
  char *name = STRING(name);
  double xscale = (double)VARG(xscale);
  double yscale = (double)VARG(yscale);
  double zscale = (double)VARG(zscale);
  double rotation = (double)VARG(rotation);
  bool is_overlay = (bool)VARG(is_overlay);
  SET_PT1 (ins_pt);
  _obj = dwg_add_INSERT (blkhdr, &ins_pt, name, xscale, yscale, zscale, rotation);
  if (_obj->block_header) {
    int error;
    Dwg_Object *blk = dwg_ref_object (THIS->dwg, _obj->block_header);
    Dwg_Object_BLOCK_HEADER *_blk = blk->tio.object->tio.BLOCK_HEADER;
    Dwg_Object *obj = dwg_obj_generic_to_object (_obj, &error);
    _blk->is_xref_resolved = 1;
    _blk->is_xref_dep = 1;
    _blk->is_xref_resolved = 256;
    _blk->xref_pname = strdup (path_name);
    _blk->blkisxref = 1;
    _blk->xrefoverlaid = is_overlay;
    _blk->xref = dwg_add_handleref (THIS->dwg, 5, obj->handle.value, blk);
    _blk->flag |= 1 << 4 | 1 << 6;
  }
  GB.ReturnObject (obj_generic_to_gb (_obj));
END_METHOD


// This is backed by block_header iterators,
// but the key is not a string, but indices or handles. Returns dwg_ent_generic.
#undef ENTITY_COLLECTION
#define ENTITY_COLLECTION(token)                                \
GB_DESC token##_Desc[] =                                        \
  {                                                             \
    GB_DECLARE(#token, sizeof(C##token)), GB_NOT_CREATABLE(),   \
    /* List of entities */                                      \
    GB_PROPERTY_READ("Count", "i", Entities_Count),             \
    /* Foreach entity type AddType */                           \
    GB_METHOD("Add3DFace", "_3DFace;", Blk_Add3DFace, "(Point1)f[3](Point2)f[3](Point3)f[3](Point4)f[3]"), \
    GB_METHOD("Add3DMesh", "_3DMesh;", Blk_Add3DMesh, "(M)i(N)i(PointsMatrix)f[16]"), \
    GB_METHOD("Add3DPoly", "_3DPoly;", Blk_Add3DPoly, "(Points)f[]"), \
    GB_METHOD("AddArc", "_Arc;", Blk_AddArc, "(Center)f[3](Radius)f(StartAngle)f(EndAngle)f"), \
    /*GB_METHOD("AddAttribute", "_ATTDEF;", Blk_AddAttribute, "(Height)f(Mode)i(Prompt)s(InsPoint)f[3](Tag)s(Value)s"),*/                                                                  \
    GB_METHOD("AddBox", "_3DSolid;", Blk_AddBox, "(Origin)f[3](Length)f(Width)f(Height)f"), \
    GB_METHOD("AddCircle", "_Circle;", Blk_AddCircle, "(Center)f[3](Radius)f"), \
    GB_METHOD("AddCone", "_3DSolid;", Blk_AddCone, "(Center)f[3](BaseRadius)f(Height)f"), \
    /*GB_METHOD("AddCustomObject", "o", Blk_AddCustomObject, "(ClasseName)s"),*/ \
    GB_METHOD("AddCylinder", "_3DSolid;", Blk_AddCylinder, "(Center)f[3](Radius)f(Height)f"), \
    GB_METHOD("AddDim3PointAngular", "_Dim3PointAngular;", Blk_AddDim3PointAngular, "(AngleVertex)f[3](FirstEndPoint)f[3](SecondEndPoint)f[3](TextPoint)f[3]"), \
    GB_METHOD("AddDimAligned", "_DimAligned;", Blk_AddDimAligned, "(FirstEndPoint)f[3](SecondEndPoint)f[3](TextPoint)f[3]"), \
    GB_METHOD("AddDimAngular", "_DimAngular;", Blk_AddDimAngular, "(CenterPoint)f[3](FirstEndPoint)f[3](SecondEndPoint)f[3](TextPoint)f[3]"), \
    GB_METHOD("AddDimArc", "_DimArc;", Blk_AddDimArc, "(Center)f[3](FirstEndPoint)f[3](SecondEndPoint)f[3](ArcPoint)f[3]"), \
    GB_METHOD("AddDimDiametric", "_DimDiametric;", Blk_AddDimDiametric, "(FirstEndPoint)f[3](SecondEndPoint)f[3](TextPoint)f[3]"), \
    GB_METHOD("AddDimOrdinate", "_DimOrdinate;", Blk_AddDimOrdinate, "(DefPoint)f[3](LeaderEndPoint)f[3](UseXAxis)b"), \
    GB_METHOD("AddDimRadial", "_DimRadial;", Blk_AddDimRadial, "(Center)f[3](ChordPoint)f[3](LeaderLength)f"), \
    GB_METHOD("AddDimRadialLarge", "_DimRadialLarge;", Blk_AddDimRadialLarge, "(Center)f[3](ChordPoint)f[3](OverrideCenter)f[3](JogPoint)f[3](JogAngle)f"), \
    GB_METHOD("AddDimRotated", "_DimRotated;", Blk_AddDimRotated, "(XLine1Point)f[3](XLine2Point)f[3](DefPoint)f[3](RotationAngle)f"), \
    GB_METHOD("AddEllipse", "_Ellipse;", Blk_AddEllipse, "(Center)f[3](MajorAxis)f(RadiusRatio)f"), \
    GB_METHOD("AddEllipticalCone", "_3DSolid;", Blk_AddEllipticalCone, "(Center)f[3](MajorRadius)f(MinorRadius)f(Height)f"), \
    GB_METHOD("AddEllipticalCylinder", "_3DSolid;", Blk_AddEllipticalCylinder, "(Center)f[3](MajorRadius)f(MinorRadius)f(Height)f"), \
    GB_METHOD("AddExtrudedSolid", "_3DSolid;", Blk_AddExtrudedSolid, "(Profile)o(Height)f(TaperAngle)f"), \
    GB_METHOD("AddExtrudedSolidAlongPath", "_3DSolid;", Blk_AddExtrudedSolidAlongPath, "(Profile)o(PathObj)o"), \
    GB_METHOD("AddHatch", "_Hatch;", Blk_AddHatch, "(PatternType)i(PatternName)s(Associativity)b(PathObjects)o[]"), \
    GB_METHOD("AddLeader", "_Leader;", Blk_AddLeader, "(Points)f[](Annotation)o(Type)i"), \
    GB_METHOD("AddLightWeightPolyline", "_LightWeightPolyline;", Blk_AddLightWeightPolyline, \
              "(Points2D)f[]"),                                         \
    GB_METHOD("AddLine", "_Line;", Blk_AddLine, "(StartPoint)f[3](EndPoint)f[3]"), \
    GB_METHOD("AddMInsertBlock", "_MInsertBlock;", Blk_AddMInsertBlock, "(InsPoint)f[3](Name)s(XScale)f(YScale)f(ZScale)f(Rotation)f(NumRows)i(NumColumns)i(RowSpacing)f(ColumnsSpacing)f"), \
    GB_METHOD("AddInsertBlock", "_BlockRef;", Blk_AddInsertBlock, "(InsPoint)f[3](Name)s(XScale)f(YScale)f(ZScale)f(Rotation)f"), \
    /* GB_METHOD("AddMLeader", "_MLeader;", Blk_AddMLeader, "(Points)f[](LeaderLineIndex)i"), */ \
    GB_METHOD("AddMLine", "_MLine;", Blk_AddMLine, "(points3d)f[]"), \
    GB_METHOD("AddMText", "_MText;", Blk_AddMText, "(InsPoint)f[3](Width)f(Text)s"), \
    GB_METHOD("AddPoint", "_Point;", Blk_AddPoint, "(Point)f[3]"), \
    GB_METHOD("AddPolyfaceMesh", "_PolyfaceMesh;", Blk_AddPolyfaceMesh, "(Points)f[](Faces)i[]"), \
    GB_METHOD("AddPolyline", "_Polyline;", Blk_AddPolyline, "(Points3D)f[]"), \
    GB_METHOD("AddPViewport", "_PViewport;", Blk_AddPViewport, "(Center)f[3](Width)f(Height)f"), \
    GB_METHOD("AddPyramid", "_3DSolid;", Blk_AddCone, "(Center)f[3]f(Radius)f(Height)f"), \
    GB_METHOD("AddRasterImage", "_RasterImage;", Blk_AddRasterImage, \
              "(FileName)s(InsPt)f[3](Scalefactor)f(RotationAngle)f"),  \
    GB_METHOD("AddRay", "_Ray;", Blk_AddRay, "(Point1)f[3](Point2)f[3]"),  \
    GB_METHOD("AddRevolvedSolid", "_3DSolid;", Blk_AddRevolvedSolid, \
              "(Profile)o(axis_pt)f[3](axis_dir)f[3](angle)f"),      \
    /*                                                               \
    GB_METHOD("AddSection", "_Section;", Blk_AddSection, "(FromPoint)f[3](ToPoint)f[3](PlaneVector)f[3]"), */ \
    GB_METHOD("AddShape", "_Shape;", Blk_AddShape, "(Name)s(ins_pt)f[3](scale_factor)f(oblique_angle)f"), \
    GB_METHOD("AddSolid", "_Solid;", Blk_AddSolid, "(pt1)f[3](pt2)f[3](pt3)f[3](pt4)f[3]"), \
    GB_METHOD("AddSphere", "_3DSolid;", Blk_AddSphere, "(Center)f[3](Radius)f"), \
    GB_METHOD("AddSpline", "_Spline;", Blk_AddSpline, "(fit_pts)f[](beg_tan_vec)f[3](end_tan_vec)f[3]"), \
    /*GB_METHOD("AddTable", "_Table;", Blk_AddTable, "(ins_pt)f[3](num_rows)i(num_cols)i(row_height)f(col_width)f"),*/ \
    GB_METHOD("AddText", "_Text;", Blk_AddText, "(text)s(ins_pt)f[3](height)f"), \
    GB_METHOD("AddTolerance", "_Tolerance;", Blk_AddTolerance, "(text)s(ins_pt)f[3](x_direction)f[3]"), \
    GB_METHOD("AddTorus", "_3DSolid;", Blk_AddTorus, "(Center)f[3](TorusRadius)f(TubeRadius)f"), \
    GB_METHOD("AddTrace", "_Trace;", Blk_AddTrace, "(points3d)f[]"), \
    GB_METHOD("AddWedge", "_3DSolid;", Blk_AddWedge, "(Center)f[3](Length)f(Width)f(Height)f"), \
    GB_METHOD("AddXLine", "_XLine;", Blk_AddXLine, "(Point1)f[3](Point2)f[3]"),  \
    GB_METHOD("AttachExternalReference", "_INSERT;", Blk_AttachExternalReference, \
              "(path_name)s(name)s(ins_pt)f[3](xscale)f(yscale)f(zscale)f(rotation)f(is_overlay)b"), \
    \
    GB_METHOD("_get", "_CDwgEntity;", Entities_get, "(Index)i"),           \
    /*GB_METHOD("_put", NULL, Entities_put, "(Object)v(Index)i"),*/        \
    GB_METHOD("_next", "_CDwgEntity;",Entities_next, NULL),                \
    GB_END_DECLARE                                                         \
  }
ENTITY_COLLECTION (ModelSpace);
ENTITY_COLLECTION (PaperSpace);
