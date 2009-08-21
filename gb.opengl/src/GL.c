/***************************************************************************

  GL.c

  (c) 2005-2007 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#define __GL_C

#include "gambas.h"
#include "gb_common.h"
#include "main.h"

#include "GL.h"

#include <GL/gl.h>

#include "GLclipping.h"
#include "GLcolorLighting.h"
#include "GLcoordTransf.h"
#include "GLdisplayList.h"
#include "GLfog.h"
#include "GLframeBufferOps.h"
#include "GLmodesExec.h"
#include "GLpixelOperations.h"
#include "GLprimitives.h"
#include "GLrasterization.h"
#include "GLtextureMapping.h"
#include "GLinfo.h"
#include "GLselectFeedback.h"

/**************************************************************************/

BEGIN_METHOD_VOID(GL_exit)

	// free reserved memories
	freeGetsAllocs();

END_METHOD

BEGIN_METHOD_VOID(GLCHECKERROR)

	GB.ReturnInteger(glGetError());

END_METHOD

/**************************************************************************/

GB_DESC Cgl[] =
{
	GB_DECLARE("Gl",0), GB_NOT_CREATABLE(),
	
//	GB_STATIC_METHOD("_init", NULL, GL_init, NULL),
	GB_STATIC_METHOD("_exit", NULL, GL_exit, NULL),

	/* Check errors */
	GB_STATIC_METHOD("GetError", "i", GLCHECKERROR, NULL),

	/* Primitives - see GLprimitives.h */
	GB_STATIC_METHOD("Begin", NULL, GLBEGIN, "(Primitive)i"),
	GB_STATIC_METHOD("EdgeFlag", NULL, GLEDGEFLAG, "(Flag)b"),
	GB_STATIC_METHOD("End", NULL, GLEND, NULL),
	GB_STATIC_METHOD("Rectf", NULL, GLRECTF, "(X1)f(Y1)f(X2)f(Y2)f"),
	GB_STATIC_METHOD("Recti", NULL, GLRECTI, "(X1)i(Y1)i(X2)i(Y2)i"),
      GB_STATIC_METHOD("Vertex2f", NULL, GLVERTEX2F, "(X)f(Y)f"),
      GB_STATIC_METHOD("Vertex3f", NULL, GLVERTEX3F, "(X)f(Y)f(Z)f"),
      GB_STATIC_METHOD("Vertex4f", NULL, GLVERTEXF, "(X)f(Y)f(Z)f(W)f"),
      GB_STATIC_METHOD("Vertexf", NULL, GLVERTEXF, "(X)f(Y)f[(Z)f(W)f]"),
      GB_STATIC_METHOD("Vertex2i", NULL, GLVERTEX2I, "(X)i(Y)i"),
      GB_STATIC_METHOD("Vertex3i", NULL, GLVERTEX3I, "(X)i(Y)i(Z)i"),
      GB_STATIC_METHOD("Vertex4i", NULL, GLVERTEXI, "(X)i(Y)i(Z)i(W)i"),
      GB_STATIC_METHOD("Vertexi", NULL, GLVERTEXI, "(X)i(Y)i[(Z)i(W)i]"),
	GB_STATIC_METHOD("Vertexfv", NULL, GLVERTEXFV, "(Coords)Float[]"),
	GB_STATIC_METHOD("Vertexiv", NULL, GLVERTEXIV, "(Coords)Integer[]"),

	/* Display lists - see GLdisplayList.h */
	GB_STATIC_METHOD("CallList", NULL, GLCALLLIST, "(Index)i"),
	GB_STATIC_METHOD("CallLists", NULL, GLCALLLISTS, "(Lists)Integer[]"),
	GB_STATIC_METHOD("DeleteLists", NULL, GLDELETELISTS, "(Index)i(Range)i"),
	GB_STATIC_METHOD("EndList", NULL, GLENDLIST, NULL),
	GB_STATIC_METHOD("GenLists", "i", GLGENLISTS, "(Count)i"),
	GB_STATIC_METHOD("IsList", "b", GLISLIST, "(Index)i"),
	GB_STATIC_METHOD("ListBase", NULL, GLLISTBASE, "(Index)i"),
	GB_STATIC_METHOD("NewList", NULL, GLNEWLIST, "(Index)i(Mode)i"),

	/* Coordinate Transformation - see GLcoordTransf.h */
	GB_STATIC_METHOD("DepthRange", NULL, GLDEPTHRANGE, "(Near)f(Far)f"),
	GB_STATIC_METHOD("Frustum", NULL, GLFRUSTUM, "(Left)f(Right)f(Bottom)f(Top)f(Near)f(Far)f"),
	GB_STATIC_METHOD("LoadIdentity", NULL, GLLOADIDENTITY, NULL),
	GB_STATIC_METHOD("LoadMatrixf", NULL, GLLOADMATRIXF, "(Matrix)Float[]"),
	GB_STATIC_METHOD("MatrixMode", NULL, GLMATRIXMODE, "(Mode)i"),
	GB_STATIC_METHOD("MultMatrixf", NULL, GLMULTMATRIXF, "(Matrix)Float[]"),
	GB_STATIC_METHOD("Ortho", NULL, GLORTHO, "(Left)f(Right)f(Bottom)f(Top)f(Near)f(Far)f"),
	GB_STATIC_METHOD("PopMatrix", NULL, GLPOPMATRIX, NULL),
	GB_STATIC_METHOD("PushMatrix", NULL, GLPUSHMATRIX, NULL),
	GB_STATIC_METHOD("Rotatef", NULL, GLROTATEF, "(Angle)f(X)f(Y)f(Z)f"),
	GB_STATIC_METHOD("Scalef", NULL, GLSCALEF, "(X)f(Y)f(Z)f"),
	GB_STATIC_METHOD("Translatef", NULL, GLTRANSLATEF, "(X)f(Y)f(Z)f"),
	GB_STATIC_METHOD("Viewport", NULL, GLVIEWPORT, "(X)i(Y)i(Width)i(Height)i"),

	/* Coloring and Lighting - see GLcolorLighting.h */
      GB_STATIC_METHOD("Color3f", NULL, GLCOLOR3F, "(Red)f(Green)f(Blue)f"),
      GB_STATIC_METHOD("Color4f", NULL, GLCOLORF, "(Red)f(Green)f(Blue)f(Alpha)f"),
      GB_STATIC_METHOD("Colorf", NULL, GLCOLORF, "(Red)f(Green)f(Blue)f[(Alpha)f]"),
      GB_STATIC_METHOD("Color3i", NULL, GLCOLOR3I, "(Red)i(Green)i(Blue)i"),
      GB_STATIC_METHOD("Color4i", NULL, GLCOLORI, "(Red)i(Green)i(Blue)i(Alpha)i"),
      GB_STATIC_METHOD("Colori", NULL, GLCOLORI, "(Red)i(Green)i(Blue)i[(Alpha)i]"),
	GB_STATIC_METHOD("Colorfv", NULL, GLCOLORFV, "(Colors)Float[]"),
	GB_STATIC_METHOD("Coloriv", NULL, GLCOLORIV, "(Colors)Integer[]"),
	GB_STATIC_METHOD("ColorMaterial", NULL, GLCOLORMATERIAL, "(Face)i(Mode)i"),
	GB_STATIC_METHOD("FrontFace", NULL, GLFRONTFACE, "(Mode)i"),
	GB_STATIC_METHOD("GetLightfv", "Float[]", GLGETLIGHTFV, "(Light)i(Pname)i"),
	GB_STATIC_METHOD("GetLightiv", "Integer[]", GLGETLIGHTIV, "(Light)i(Pname)i"),
	GB_STATIC_METHOD("GetMaterialfv", "Float[]", GLGETMATERIALFV, "(Face)i(Pname)i"),
	GB_STATIC_METHOD("GetMaterialiv", "Integer[]", GLGETMATERIALIV, "(Face)i(Pname)i"),
	GB_STATIC_METHOD("Indexf", NULL, GLINDEXF, "(Index)f"),
	GB_STATIC_METHOD("Indexi", NULL, GLINDEXI, "(Index)i"),
	GB_STATIC_METHOD("Lightf", NULL, GLLIGHTF, "(Light)i(Pname)i(Param)f"),
	GB_STATIC_METHOD("Lighti", NULL, GLLIGHTI, "(Light)i(Pname)i(Param)i"),
	GB_STATIC_METHOD("Lightfv", NULL, GLLIGHTFV, "(Light)i(Pname)i(Params)Float[]"),
	GB_STATIC_METHOD("Lightiv", NULL, GLLIGHTIV, "(Light)i(Pname)i(Params)Integer[]"),
	GB_STATIC_METHOD("LightModelf", NULL, GLLIGHTMODELF, "(Pname)i(Param)f"),
	GB_STATIC_METHOD("LightModeli", NULL, GLLIGHTMODELI, "(Pname)i(Param)i"),
	GB_STATIC_METHOD("LightModelfv", NULL, GLLIGHTMODELFV, "(Pname)i(Params)Float[]"),
	GB_STATIC_METHOD("LightModeliv", NULL, GLLIGHTMODELIV, "(Pname)i(Params)Integer[]"),
	GB_STATIC_METHOD("Materialf", NULL, GLMATERIALF, "(Face)i(Pname)i(Param)f"),
	GB_STATIC_METHOD("Materiali", NULL, GLMATERIALI, "(Face)i(Pname)i(Param)i"),
	GB_STATIC_METHOD("Materialfv", NULL, GLMATERIALFV, "(Face)i(Pname)i(Params)Float[]"),
	GB_STATIC_METHOD("Materialiv", NULL, GLMATERIALIV, "(Face)i(Pname)i(Params)Integer[]"),
	GB_STATIC_METHOD("Normal3f", NULL, GLNORMAL3F, "(Nx)f(Ny)f(Nz)f"),
	GB_STATIC_METHOD("Normal3i", NULL, GLNORMAL3I, "(Nx)i(Ny)i(Nz)i"),
	GB_STATIC_METHOD("Normal3fv", NULL, GLNORMAL3FV, "(Params)Float[]"),
	GB_STATIC_METHOD("Normal3iv", NULL, GLNORMAL3IV, "(Params)Integer[]"),
	GB_STATIC_METHOD("ShadeModel", NULL, GLSHADEMODEL, "(Model)i"),

	/* Clipping - see GLclipping.h */
	GB_STATIC_METHOD("ClipPlane", NULL, GLCLIPPLANE, "(Plane)i(Equation)Float[]"),
	GB_STATIC_METHOD("GetClipPlane", "Float[]", GLGETCLIPPLANE, "(Plane)i"),

	/* Rasterization - see GLrasterization.h  */
	//GB_STATIC_METHOD("Bitmap", NULL, GLBITMAP, NULL), //TODO
	GB_STATIC_METHOD("CullFace", NULL, GLCULLFACE, "(Mode)i"),
	//GB_STATIC_METHOD("GetPolygonStipple", "i", GLPOLYGONSTIPPLE, NULL), //TODO
	GB_STATIC_METHOD("LineStipple", NULL, GLLINESTIPPLE, "(Factor)i(Pattern)i"),
	GB_STATIC_METHOD("LineWidth", NULL, GLLINEWIDTH, "(Width)f"),
	GB_STATIC_METHOD("PointSize", NULL, GLPOINTSIZE, "(Size)f"),
	GB_STATIC_METHOD("PolygonMode", NULL, GLPOLYGONMODE, "(Face)i(Mode)i"),
	//GB_STATIC_METHOD("PolygonStipple", NULL, GLPOLYGONSTIPPLE, "(Mask)i"), //TODO
      GB_STATIC_METHOD("RasterPos2f", NULL, GLRASTERPOS2F, "(X)f(Y)f"),
      GB_STATIC_METHOD("RasterPos3f", NULL, GLRASTERPOS3F, "(X)f(Y)f(Z)f"),
      GB_STATIC_METHOD("RasterPos4f", NULL, GLRASTERPOSF, "(X)f(Y)f(Z)f(W)f"),
      GB_STATIC_METHOD("RasterPosf", NULL, GLRASTERPOSF, "(X)f(Y)f[(Z)f(W)f]"),
      GB_STATIC_METHOD("RasterPos2i", NULL, GLRASTERPOS2I, "(X)i(Y)i"),
      GB_STATIC_METHOD("RasterPos3i", NULL, GLRASTERPOS3I, "(X)i(Y)i(Z)i"),
      GB_STATIC_METHOD("RasterPos4i", NULL, GLRASTERPOSI, "(X)i(Y)i(Z)i(W)i"),
      GB_STATIC_METHOD("RasterPosi", NULL, GLRASTERPOSI, "(X)i(Y)i[(Z)i(W)i]"),
	GB_STATIC_METHOD("RasterPosfv", NULL, GLRASTERPOSFV, "(Coords)Float[]"),
	GB_STATIC_METHOD("RasterPosiv", NULL, GLRASTERPOSIV, "(Coords)Integer[]"),
	
	/* PixelOperations - see GLpixelOperations.h  */
	// TODO glReadPixels
	GB_STATIC_METHOD("CopyPixels", NULL, GLCOPYPIXELS, "(X)i(Y)i(Width)i(Height)i(Buffer)i"),
	GB_STATIC_METHOD("PixelStoref", NULL, GLPIXELSTOREF, "(Pname)i(Param)f"),
	GB_STATIC_METHOD("PixelStorei", NULL, GLPIXELSTOREI, "(Pname)i(Param)i"),
	GB_STATIC_METHOD("PixelTransferf", NULL, GLPIXELTRANSFERF, "(Pname)i(Param)f"),
	GB_STATIC_METHOD("PixelTransferi", NULL, GLPIXELTRANSFERI, "(Pname)i(Param)i"),
	GB_STATIC_METHOD("ReadBuffer", NULL, GLREADBUFFER, "(Mode)i"),
	GB_STATIC_METHOD("DrawPixels", NULL, GLDRAWPIXELS, "(Image)Image;"),

	/* Modes and Execution - see GLmodesExec.h */
	GB_STATIC_METHOD("Disable", NULL, GLDISABLE, "(Capacity)i"),
	GB_STATIC_METHOD("Enable", NULL, GLENABLE, "(Capacity)i"),
	GB_STATIC_METHOD("Flush", NULL, GLFLUSH, NULL),
	GB_STATIC_METHOD("Finish", NULL, GLFINISH, NULL),
	GB_STATIC_METHOD("Hint", NULL, GLHINT, "(Target)i(Mode)i"),
	GB_STATIC_METHOD("IsEnabled", "b", GLISENABLED, "(Capacity)i"),

	/* Fog - see GLfog.h */
	GB_STATIC_METHOD("Fogf", NULL, GLFOGF, "(Pname)i(Param)f"),
	GB_STATIC_METHOD("Fogi", NULL, GLFOGI, "(Pname)i(Param)i"),
	GB_STATIC_METHOD("Fogfv", NULL, GLFOGFV, "(Pname)i(Params)Float[]"),
	GB_STATIC_METHOD("Fogiv", NULL, GLFOGIV, "(Pname)i(Params)Integer[]"),

	/* Texture Mapping - see GLtextureMapping.h */
	GB_STATIC_METHOD("BindTexture", NULL, GLBINDTEXTURE, "(Target)i(Texture)i"),
	// TODO adapt to gambas
	//GB_STATIC_METHOD("CopyTexImage1D", NULL, GLCOPYTEXIMAGE1D, "(Target)i(Level)i(Format)i(X)i(Y)i(Width)i(Border)i"),
	//GB_STATIC_METHOD("CopyTexImage2D", NULL, GLCOPYTEXIMAGE2D, "(Target)i(Level)i(Format)i(X)i(Y)i(Width)i(Height)i(Border)i"),
	GB_STATIC_METHOD("DeleteTextures", NULL, GLDELETETEXTURES, "(Textures)Integer[]"),
	GB_STATIC_METHOD("GenTextures", "Integer[]", GLGENTEXTURES, "(Count)i"),
	GB_STATIC_METHOD("IsTexture", "b", GLISTEXTURE, "(Texture)i"),
      GB_STATIC_METHOD("TexCoord1f", NULL, GLTEXCOORD1F, "(S)f"),
      GB_STATIC_METHOD("TexCoord2f", NULL, GLTEXCOORD2F, "(S)f(T)f"),
      GB_STATIC_METHOD("TexCoord3f", NULL, GLTEXCOORD3F, "(S)f(T)f(R)f"),
      GB_STATIC_METHOD("TexCoord4f", NULL, GLTEXCOORDF, "(S)f(T)f(R)f(Q)f"),
      GB_STATIC_METHOD("TexCoordf", NULL, GLTEXCOORDF, "(S)f[(T)f(R)f(Q)f]"),
      GB_STATIC_METHOD("TexCoord1i", NULL, GLTEXCOORD1I, "(S)i"),
      GB_STATIC_METHOD("TexCoord2i", NULL, GLTEXCOORD2I, "(S)i(T)i"),
      GB_STATIC_METHOD("TexCoord3i", NULL, GLTEXCOORD3I, "(S)i(T)i(R)i"),
      GB_STATIC_METHOD("TexCoord4i", NULL, GLTEXCOORDI, "(S)i(T)i(R)i(Q)i"),
      GB_STATIC_METHOD("TexCoordi", NULL, GLTEXCOORDI, "(S)i[(T)i(R)i(Q)i]"),
	GB_STATIC_METHOD("TexEnvf", NULL, GLTEXENVF, "(Target)i(Pname)i(Param)f"),
	GB_STATIC_METHOD("TexEnvfv", NULL, GLTEXENVFV, "(Target)i(Pname)i(Params)Float[]"),
	GB_STATIC_METHOD("TexEnvi", NULL, GLTEXENVI, "(Target)i(Pname)i(Param)i"),
	GB_STATIC_METHOD("TexEnviv", NULL, GLTEXENVIV, "(Target)i(Pname)i(Params)Integer[]"),
	GB_STATIC_METHOD("TexImage1D", NULL, GLTEXIMAGE1D, "(Image)Image;[(Level)i(Border)i]"),
	GB_STATIC_METHOD("TexImage2D", NULL, GLTEXIMAGE2D,"(Image)Image;[(Level)i(Border)i]"),
	GB_STATIC_METHOD("TexParameterf", NULL, GLTEXPARAMETERF, "(Target)i(Pname)i(Param)f"),
	GB_STATIC_METHOD("TexParameterfv", NULL, GLTEXPARAMETERFV, "(Target)i(Pname)i(Params)Float[]"),
	GB_STATIC_METHOD("TexParameteri", NULL, GLTEXPARAMETERI, "(Target)i(Pname)i(Param)i"),
	GB_STATIC_METHOD("TexParameteriv", NULL, GLTEXPARAMETERIV, "(Target)i(Pname)i(Params)Integer[]"),

	/* FrameBuffer Operations - see GLframeBufferOps.h  */
	GB_STATIC_METHOD("Accum", NULL, GLACCUM, "(Operation)i(Value)f"),
	GB_STATIC_METHOD("AlphaFunc", NULL, GLALPHAFUNC, "(Function)i(Reference)f"),
	GB_STATIC_METHOD("BlendFunc", NULL, GLBLENDFUNC, "(SrcFactor)i(DstFactor)i"),
	GB_STATIC_METHOD("Clear", NULL, GLCLEAR, "(Mask)i"),
	GB_STATIC_METHOD("ClearAccum", NULL, GLCLEARACCUM, "(Red)f(Green)f(Blue)f(Alpha)f"),
	GB_STATIC_METHOD("ClearColor", NULL, GLCLEARCOLOR, "(Red)f(Green)f(Blue)f(Alpha)f"),
	GB_STATIC_METHOD("ClearDepth", NULL, GLCLEARDEPTH, "(Depth)f"),
	GB_STATIC_METHOD("ClearIndex", NULL, GLCLEARINDEX, "(Value)f"),
	GB_STATIC_METHOD("ClearStencil", NULL, GLCLEARSTENCIL, "(Value)i"),
	GB_STATIC_METHOD("ColorMask", NULL, GLCOLORMASK, "(Red)b(Green)b(Blue)b(Alpha)b"),
	GB_STATIC_METHOD("DrawBuffer", NULL, GLDRAWBUFFER, "(Mode)i"),
	GB_STATIC_METHOD("DepthFunc", NULL, GLDEPTHFUNC, "(Function)i"),
	GB_STATIC_METHOD("DepthMask", NULL, GLDEPTHMASK, "(Flag)b"),
	GB_STATIC_METHOD("IndexMask", NULL, GLINDEXMASK, "(Mask)i"),
	GB_STATIC_METHOD("LogicOp", NULL, GLLOGICOP, "(Opcode)i"),
	GB_STATIC_METHOD("Scissor", NULL, GLSCISSOR, "(X)i(Y)i(Width)i(Height)i"),
	GB_STATIC_METHOD("StencilFunc", NULL, GLSTENCILFUNC, "(Function)i(Reference)i(Mask)i"),
	GB_STATIC_METHOD("StencilMask", NULL, GLSTENCILMASK, "(Mask)i"),
	GB_STATIC_METHOD("StencilOp", NULL, GLSTENCILOP, "(Fail)i(Zfail)i(Zpass)i"),

	/* Selection and Feedback - see GLselectPixmap.h  */
      GB_STATIC_METHOD("FeedbackBuffer", NULL, GLFEEDBACKBUFFER, "(Type)i"),
      GB_STATIC_METHOD("InitNames", NULL, GLINITNAMES, NULL),
      GB_STATIC_METHOD("LoadName", NULL, GLLOADNAME, "(Name)i"),
      GB_STATIC_METHOD("PassThrough", NULL, GLPASSTHROUGH, "(Token)f"),
      GB_STATIC_METHOD("PopName", NULL, GLPOPNAME, NULL),
      GB_STATIC_METHOD("PushName", NULL, GLPUSHNAME, "(Name)i"),
	GB_STATIC_METHOD("RenderMode", "Object[];", GLRENDERMODE, "(Mode)i"),
      GB_STATIC_METHOD("SelectBuffer", NULL, GLSELECTBUFFER, NULL),

	/* glGetxxxx calls - see GLinfo.h/c   */
	GB_STATIC_METHOD("GetBooleanv", "Boolean[];", GLGETBOOLEANV, "(Parameter)i[(Size)i]"),
	GB_STATIC_METHOD("GetFloatv", "Float[];", GLGETFLOATV, "(Parameter)i[(Size)i]"),
	GB_STATIC_METHOD("GetIntegerv", "Integer[];", GLGETINTEGERV, "(Parameter)i[(Size)i]"),

	/********************/
	/* opengl constants */
	/********************/

	/* Primitives */
	GB_CONSTANT("GL_POINTS", "i", GL_POINTS),
	GB_CONSTANT("GL_LINES", "i", GL_LINES),
	GB_CONSTANT("GL_LINE_LOOP", "i", GL_LINE_LOOP),
	GB_CONSTANT("GL_LINE_STRIP", "i", GL_LINE_STRIP),
	GB_CONSTANT("GL_TRIANGLES", "i", GL_TRIANGLES),
	GB_CONSTANT("GL_TRIANGLE_STRIP", "i", GL_TRIANGLE_STRIP),
	GB_CONSTANT("GL_TRIANGLE_FAN", "i", GL_TRIANGLE_FAN),
	GB_CONSTANT("GL_QUADS", "i", GL_QUADS),
	GB_CONSTANT("GL_QUAD_STRIP", "i", GL_QUAD_STRIP),
	GB_CONSTANT("GL_POLYGON", "i", GL_POLYGON),

	/* Matrix Mode */
	GB_CONSTANT("GL_MATRIX_MODE", "i", GL_MATRIX_MODE),
	GB_CONSTANT("GL_MODELVIEW", "i", GL_MODELVIEW),
	GB_CONSTANT("GL_PROJECTION", "i", GL_PROJECTION),
	GB_CONSTANT("GL_TEXTURE", "i", GL_TEXTURE),

	/* Points */
	GB_CONSTANT("GL_POINT_SMOOTH", "i", GL_POINT_SMOOTH),
	GB_CONSTANT("GL_POINT_SIZE", "i", GL_POINT_SIZE),
	GB_CONSTANT("GL_POINT_SIZE_GRANULARITY", "i", GL_POINT_SIZE_GRANULARITY),
	GB_CONSTANT("GL_POINT_SIZE_RANGE", "i", GL_POINT_SIZE_RANGE),

	/* Display Lists */
	GB_CONSTANT("GL_COMPILE", "i", GL_COMPILE),
	GB_CONSTANT("GL_COMPILE_AND_EXECUTE", "i", GL_COMPILE_AND_EXECUTE),
	GB_CONSTANT("GL_LIST_BASE", "i", GL_LIST_BASE),
	GB_CONSTANT("GL_LIST_INDEX", "i", GL_LIST_INDEX),
	GB_CONSTANT("GL_LIST_MODE" ,"i", GL_LIST_MODE),

	/* Accumulation buffer */
	GB_CONSTANT("GL_ACCUM", "i", GL_ACCUM),
	GB_CONSTANT("GL_ACCUM_ALPHA_BITS", "i", GL_ACCUM_ALPHA_BITS),
	GB_CONSTANT("GL_ACCUM_CLEAR_VALUE", "i", GL_ACCUM_CLEAR_VALUE),
	GB_CONSTANT("GL_ACCUM_BLUE_BITS", "i", GL_ACCUM_BLUE_BITS),
	GB_CONSTANT("GL_ACCUM_GREEN_BITS", "i", GL_ACCUM_GREEN_BITS),
	GB_CONSTANT("GL_ACCUM_RED_BITS", "i", GL_ACCUM_RED_BITS),
	GB_CONSTANT("GL_ADD", "i", GL_ADD),
	GB_CONSTANT("GL_LOAD", "i", GL_LOAD),
	GB_CONSTANT("GL_MULT", "i", GL_MULT),
	GB_CONSTANT("GL_RETURN", "i", GL_RETURN),

	/* User clipping planes */
	GB_CONSTANT("GL_CLIP_PLANE0", "i", GL_CLIP_PLANE0),
	GB_CONSTANT("GL_CLIP_PLANE1", "i", GL_CLIP_PLANE1),
	GB_CONSTANT("GL_CLIP_PLANE2", "i", GL_CLIP_PLANE2),
	GB_CONSTANT("GL_CLIP_PLANE3", "i", GL_CLIP_PLANE3),
	GB_CONSTANT("GL_CLIP_PLANE4", "i", GL_CLIP_PLANE4),
	GB_CONSTANT("GL_CLIP_PLANE5", "i", GL_CLIP_PLANE5),

	/* Alpha testing */
	GB_CONSTANT("GL_ALPHA_TEST", "i", GL_ALPHA_TEST),
	GB_CONSTANT("GL_ALPHA_TEST_REF", "i", GL_ALPHA_TEST_REF),
	GB_CONSTANT("GL_ALPHA_TEST_FUNC", "i", GL_ALPHA_TEST_FUNC),

	/* glPush/PopAttrib bits */
	GB_CONSTANT("GL_ACCUM_BUFFER_BIT", "i", GL_ACCUM_BUFFER_BIT),
	GB_CONSTANT("GL_ALL_ATTRIB_BITS", "i", GL_ALL_ATTRIB_BITS),
	GB_CONSTANT("GL_CURRENT_BIT", "i", GL_CURRENT_BIT),
	GB_CONSTANT("GL_COLOR_BUFFER_BIT", "i", GL_COLOR_BUFFER_BIT),
	GB_CONSTANT("GL_DEPTH_BUFFER_BIT", "i", GL_DEPTH_BUFFER_BIT),
	GB_CONSTANT("GL_ENABLE_BIT", "i", GL_ENABLE_BIT),
	GB_CONSTANT("GL_EVAL_BIT", "i", GL_EVAL_BIT),
	GB_CONSTANT("GL_FOG_BIT", "i", GL_FOG_BIT),
	GB_CONSTANT("GL_HINT_BIT", "i", GL_HINT_BIT),
	GB_CONSTANT("GL_LIGHTING_BIT", "i", GL_LIGHTING_BIT),
	GB_CONSTANT("GL_LINE_BIT", "i", GL_LINE_BIT),
	GB_CONSTANT("GL_LIST_BIT", "i", GL_LIST_BIT),
	GB_CONSTANT("GL_PIXEL_MODE_BIT", "i", GL_PIXEL_MODE_BIT),
	GB_CONSTANT("GL_POINT_BIT", "i", GL_POINT_BIT),
	GB_CONSTANT("GL_POLYGON_BIT", "i", GL_POLYGON_BIT),
	GB_CONSTANT("GL_POLYGON_STIPPLE_BIT", "i", GL_POLYGON_STIPPLE_BIT),
	GB_CONSTANT("GL_SCISSOR_BIT", "i", GL_SCISSOR_BIT),
	GB_CONSTANT("GL_STENCIL_BUFFER_BIT", "i", GL_STENCIL_BUFFER_BIT),
	GB_CONSTANT("GL_TEXTURE_BIT", "i", GL_TEXTURE_BIT),
	GB_CONSTANT("GL_TRANSFORM_BIT", "i", GL_TRANSFORM_BIT),
	GB_CONSTANT("GL_VIEWPORT_BIT", "i", GL_VIEWPORT_BIT),

	/* Polygons */
	GB_CONSTANT("GL_POINT", "i", GL_POINT),
	GB_CONSTANT("GL_LINE", "i", GL_LINE),
	GB_CONSTANT("GL_FILL", "i", GL_FILL),
	GB_CONSTANT("GL_CW", "i", GL_CW),
	GB_CONSTANT("GL_CCW", "i", GL_CCW),
	GB_CONSTANT("GL_FRONT", "i", GL_FRONT),
	GB_CONSTANT("GL_BACK", "i", GL_BACK),
	GB_CONSTANT("GL_POLYGON_MODE", "i", GL_POLYGON_MODE),
	GB_CONSTANT("GL_POLYGON_SMOOTH", "i", GL_POLYGON_SMOOTH),
	GB_CONSTANT("GL_POLYGON_STIPPLE", "i", GL_POLYGON_STIPPLE),
	GB_CONSTANT("GL_EDGE_FLAG", "i", GL_EDGE_FLAG),
	GB_CONSTANT("GL_CULL_FACE", "i", GL_CULL_FACE),
	GB_CONSTANT("GL_CULL_FACE_MODE", "i", GL_CULL_FACE_MODE),
	GB_CONSTANT("GL_FRONT_FACE", "i", GL_FRONT_FACE),
#if 0
	// ogl 1.1 : void glPolygonOffset( GLfloat factor,GLfloat units )
	GB_CONSTANT("GL_POLYGON_OFFSET_FACTOR", "i", GL_POLYGON_OFFSET_FACTOR),
	GB_CONSTANT("GL_POLYGON_OFFSET_UNITS", "i", GL_POLYGON_OFFSET_UNITS),
	GB_CONSTANT("GL_POLYGON_OFFSET_POINT", "i", GL_POLYGON_OFFSET_POINT),
	GB_CONSTANT("GL_POLYGON_OFFSET_LINE", "i", GL_POLYGON_OFFSET_LINE),
	GB_CONSTANT("GL_POLYGON_OFFSET_FILL", "i", GL_POLYGON_OFFSET_FILL),
#endif

	/* Depth buffer */
	GB_CONSTANT("GL_NEVER", "i", GL_NEVER),
	GB_CONSTANT("GL_LESS", "i", GL_LESS),
	GB_CONSTANT("GL_EQUAL", "i", GL_EQUAL),
	GB_CONSTANT("GL_LEQUAL", "i", GL_LEQUAL),
	GB_CONSTANT("GL_GREATER", "i", GL_GREATER),
	GB_CONSTANT("GL_NOTEQUAL", "i", GL_NOTEQUAL),
	GB_CONSTANT("GL_GEQUAL", "i", GL_GEQUAL),
	GB_CONSTANT("GL_ALWAYS", "i", GL_ALWAYS),
	GB_CONSTANT("GL_DEPTH_TEST", "i", GL_DEPTH_TEST),
	GB_CONSTANT("GL_DEPTH_BITS", "i", GL_DEPTH_BITS),
	GB_CONSTANT("GL_DEPTH_CLEAR_VALUE", "i", GL_DEPTH_CLEAR_VALUE),
	GB_CONSTANT("GL_DEPTH_FUNC", "i", GL_DEPTH_FUNC),
	GB_CONSTANT("GL_DEPTH_RANGE", "i", GL_DEPTH_RANGE),
	GB_CONSTANT("GL_DEPTH_WRITEMASK", "i", GL_DEPTH_WRITEMASK),
	GB_CONSTANT("GL_DEPTH_COMPONENT", "i", GL_DEPTH_COMPONENT),

	/* Lines */
	GB_CONSTANT("GL_LINE_SMOOTH", "i", GL_LINE_SMOOTH),
	GB_CONSTANT("GL_LINE_STIPPLE", "i", GL_LINE_STIPPLE),
	GB_CONSTANT("GL_LINE_STIPPLE_PATTERN", "i", GL_LINE_STIPPLE_PATTERN),
	GB_CONSTANT("GL_LINE_STIPPLE_REPEAT", "i", GL_LINE_STIPPLE_REPEAT),
	GB_CONSTANT("GL_LINE_WIDTH", "i", GL_LINE_WIDTH),
	GB_CONSTANT("GL_LINE_WIDTH_GRANULARITY", "i", GL_LINE_WIDTH_GRANULARITY),
	GB_CONSTANT("GL_LINE_WIDTH_RANGE", "i", GL_LINE_WIDTH_RANGE),

	/* Render Mode */
	GB_CONSTANT("GL_FEEDBACK", "i", GL_FEEDBACK),
	GB_CONSTANT("GL_RENDER", "i", GL_RENDER),
	GB_CONSTANT("GL_SELECT", "i", GL_SELECT),

	/* Feedback */
	GB_CONSTANT("GL_2D", "i", GL_2D),
	GB_CONSTANT("GL_3D", "i", GL_3D),
	GB_CONSTANT("GL_3D_COLOR", "i", GL_3D_COLOR),
	GB_CONSTANT("GL_3D_COLOR_TEXTURE", "i",  GL_3D_COLOR_TEXTURE),
	GB_CONSTANT("GL_4D_COLOR_TEXTURE", "i",  GL_4D_COLOR_TEXTURE),
	GB_CONSTANT("GL_POINT_TOKEN", "i", GL_POINT_TOKEN),
	GB_CONSTANT("GL_LINE_TOKEN", "i", GL_LINE_TOKEN),
	GB_CONSTANT("GL_LINE_RESET_TOKEN", "i", GL_LINE_RESET_TOKEN),
	GB_CONSTANT("GL_POLYGON_TOKEN", "i", GL_POLYGON_TOKEN),
	GB_CONSTANT("GL_BITMAP_TOKEN", "i", GL_BITMAP_TOKEN),
	GB_CONSTANT("GL_DRAW_PIXEL_TOKEN", "i", GL_DRAW_PIXEL_TOKEN),
	GB_CONSTANT("GL_COPY_PIXEL_TOKEN", "i", GL_COPY_PIXEL_TOKEN),
	GB_CONSTANT("GL_PASS_THROUGH_TOKEN", "i", GL_PASS_THROUGH_TOKEN),
	GB_CONSTANT("GL_FEEDBACK_BUFFER_POINTER", "i", GL_FEEDBACK_BUFFER_POINTER),
	GB_CONSTANT("GL_FEEDBACK_BUFFER_SIZE", "i", GL_FEEDBACK_BUFFER_SIZE),
	GB_CONSTANT("GL_FEEDBACK_BUFFER_TYPE", "i", GL_FEEDBACK_BUFFER_TYPE),

	/* Selection */
	GB_CONSTANT("GL_SELECTION_BUFFER_POINTER", "i", GL_SELECTION_BUFFER_POINTER),
	GB_CONSTANT("GL_SELECTION_BUFFER_SIZE", "i", GL_SELECTION_BUFFER_SIZE),

	/* Fog */
	GB_CONSTANT("GL_FOG", "i", GL_FOG),
	GB_CONSTANT("GL_FOG_MODE", "i", GL_FOG_MODE),
	GB_CONSTANT("GL_FOG_DENSITY", "i", GL_FOG_DENSITY),
	GB_CONSTANT("GL_FOG_COLOR", "i", GL_FOG_COLOR),
	GB_CONSTANT("GL_FOG_INDEX", "i", GL_FOG_INDEX),
	GB_CONSTANT("GL_FOG_START", "i", GL_FOG_START),
	GB_CONSTANT("GL_FOG_END", "i", GL_FOG_END),
	GB_CONSTANT("GL_LINEAR", "i", GL_LINEAR),
	GB_CONSTANT("GL_EXP", "i", GL_EXP),
	GB_CONSTANT("GL_EXP2", "i", GL_EXP2),

	/* Errors */
	GB_CONSTANT("GL_NO_ERROR", "i", GL_NO_ERROR),
	GB_CONSTANT("GL_INVALID_VALUE", "i", GL_INVALID_VALUE),
	GB_CONSTANT("GL_INVALID_ENUM", "i", GL_INVALID_ENUM),
	GB_CONSTANT("GL_INVALID_OPERATION", "i", GL_INVALID_OPERATION),
	GB_CONSTANT("GL_STACK_OVERFLOW", "i", GL_STACK_OVERFLOW),
	GB_CONSTANT("GL_STACK_UNDERFLOW", "i", GL_STACK_UNDERFLOW),
	GB_CONSTANT("GL_OUT_OF_MEMORY", "i", GL_OUT_OF_MEMORY),

	/* Lighting */
	GB_CONSTANT("GL_LIGHTING", "i", GL_LIGHTING),
	GB_CONSTANT("GL_LIGHT0", "i", GL_LIGHT0),
	GB_CONSTANT("GL_LIGHT1", "i", GL_LIGHT1),
	GB_CONSTANT("GL_LIGHT2", "i", GL_LIGHT2),
	GB_CONSTANT("GL_LIGHT3", "i", GL_LIGHT3),
	GB_CONSTANT("GL_LIGHT4", "i", GL_LIGHT4),
	GB_CONSTANT("GL_LIGHT5", "i", GL_LIGHT5),
	GB_CONSTANT("GL_LIGHT6", "i", GL_LIGHT6),
	GB_CONSTANT("GL_LIGHT7", "i", GL_LIGHT7),
	GB_CONSTANT("GL_SPOT_EXPONENT", "i", GL_SPOT_EXPONENT),
	GB_CONSTANT("GL_SPOT_CUTTOFF", "i", GL_SPOT_CUTOFF),
	GB_CONSTANT("GL_CONSTANT_ATTENUATION", "i", GL_CONSTANT_ATTENUATION),
	GB_CONSTANT("GL_LINEAR_ATTENUATION", "i", GL_LINEAR_ATTENUATION),
	GB_CONSTANT("GL_QUADRATIC_ATTENUATION", "i", GL_QUADRATIC_ATTENUATION),
	GB_CONSTANT("GL_AMBIENT", "i", GL_AMBIENT),
	GB_CONSTANT("GL_DIFFUSE", "i", GL_DIFFUSE),
	GB_CONSTANT("GL_SPECULAR", "i", GL_SPECULAR),
	GB_CONSTANT("GL_SHININESS", "i", GL_SHININESS),
	GB_CONSTANT("GL_EMISSION", "i", GL_EMISSION),
	GB_CONSTANT("GL_POSITION", "i", GL_POSITION),
	GB_CONSTANT("GL_SPOT_DIRECTION", "i", GL_SPOT_DIRECTION),
	GB_CONSTANT("GL_AMBIENT_AND_DIFFUSE", "i", GL_AMBIENT_AND_DIFFUSE),
	GB_CONSTANT("GL_COLOR_INDEXES", "i", GL_COLOR_INDEXES),
	GB_CONSTANT("GL_LIGHT_MODEL_TWO_SIDE", "i", GL_LIGHT_MODEL_TWO_SIDE),
	GB_CONSTANT("GL_LIGHT_MODEL_LOCAL_VIEWER", "i", GL_LIGHT_MODEL_LOCAL_VIEWER),
	GB_CONSTANT("GL_LIGHT_MODEL_AMBIENT", "i", GL_LIGHT_MODEL_AMBIENT),
	GB_CONSTANT("GL_FRONT_AND_BACK", "i", GL_FRONT_AND_BACK),
	GB_CONSTANT("GL_SHADE_MODEL", "i", GL_SHADE_MODEL),
	GB_CONSTANT("GL_FLAT", "i", GL_FLAT),
	GB_CONSTANT("GL_SMOOTH", "i", GL_SMOOTH),
	GB_CONSTANT("GL_COLOR_MATERIAL", "i", GL_COLOR_MATERIAL),
	GB_CONSTANT("GL_COLOR_MATERIAL_FACE", "i", GL_COLOR_MATERIAL_FACE),
	GB_CONSTANT("GL_COLOR_MATERIAL_PARAMETER", "i", GL_COLOR_MATERIAL_PARAMETER),
	GB_CONSTANT("GL_NORMALIZE", "i", GL_NORMALIZE),

	/* Blending */
	GB_CONSTANT("GL_BLEND", "i", GL_BLEND),
	GB_CONSTANT("GL_BLEND_SRC", "i", GL_BLEND_SRC),
	GB_CONSTANT("GL_BLEND_DST", "i", GL_BLEND_DST),
	GB_CONSTANT("GL_ZERO", "i", GL_ZERO),
	GB_CONSTANT("GL_ONE", "i", GL_ONE),
	GB_CONSTANT("GL_SRC_COLOR", "i", GL_SRC_COLOR),
	GB_CONSTANT("GL_ONE_MINUS_SRC_COLOR", "i", GL_ONE_MINUS_SRC_COLOR),
	GB_CONSTANT("GL_SRC_ALPHA", "i", GL_SRC_ALPHA),
	GB_CONSTANT("GL_ONE_MINUS_SRC_ALPHA", "i", GL_ONE_MINUS_SRC_ALPHA),
	GB_CONSTANT("GL_DST_ALPHA", "i", GL_DST_ALPHA),
	GB_CONSTANT("GL_ONE_MINUS_DST_ALPHA", "i", GL_ONE_MINUS_DST_ALPHA),
	GB_CONSTANT("GL_DST_COLOR", "i", GL_DST_COLOR),
	GB_CONSTANT("GL_ONE_MINUS_DST_COLOR", "i", GL_ONE_MINUS_DST_COLOR),
	GB_CONSTANT("GL_SRC_ALPHA_SATURATE", "i", GL_SRC_ALPHA_SATURATE),

	/* Logic Ops */
	GB_CONSTANT("GL_LOGIC_OP", "i", GL_LOGIC_OP),
	GB_CONSTANT("GL_INDEX_LOGIC_OP", "i", GL_INDEX_LOGIC_OP),
	GB_CONSTANT("GL_COLOR_LOGIC_OP", "i", GL_COLOR_LOGIC_OP),
	GB_CONSTANT("GL_LOGIC_OP_MODE", "i", GL_LOGIC_OP_MODE),
	GB_CONSTANT("GL_CLEAR", "i", GL_CLEAR),
	GB_CONSTANT("GL_SET", "i", GL_SET),
	GB_CONSTANT("GL_COPY", "i", GL_COPY),
	GB_CONSTANT("GL_COPY_INVERTED", "i", GL_COPY_INVERTED),
	GB_CONSTANT("GL_NOOP", "i", GL_NOOP),
	GB_CONSTANT("GL_INVERT", "i", GL_INVERT),
	GB_CONSTANT("GL_AND", "i", GL_AND),
	GB_CONSTANT("GL_NAND", "i", GL_NAND),
	GB_CONSTANT("GL_OR", "i", GL_OR),
	GB_CONSTANT("GL_NOR", "i", GL_NOR),
	GB_CONSTANT("GL_XOR", "i", GL_XOR),
	GB_CONSTANT("GL_EQUIV", "i", GL_EQUIV),
	GB_CONSTANT("GL_AND_REVERSE", "i", GL_AND_REVERSE),
	GB_CONSTANT("GL_AND_INVERTED", "i", GL_AND_INVERTED),
	GB_CONSTANT("GL_OR_REVERSE", "i", GL_OR_REVERSE),
	GB_CONSTANT("GL_OR_INVERTED", "i", GL_OR_INVERTED),

	/* Stencil */
	GB_CONSTANT("GL_STENCIL_BITS", "i", GL_STENCIL_BITS),
	GB_CONSTANT("GL_STENCIL_TEST", "i", GL_STENCIL_TEST),
	GB_CONSTANT("GL_STENCIL_CLEAR_VALUE", "i", GL_STENCIL_CLEAR_VALUE),
	GB_CONSTANT("GL_STENCIL_FUNC", "i", GL_STENCIL_FUNC),
	GB_CONSTANT("GL_STENCIL_VALUE_MASK", "i", GL_STENCIL_VALUE_MASK),
	GB_CONSTANT("GL_STENCIL_FAIL", "i", GL_STENCIL_FAIL),
	GB_CONSTANT("GL_STENCIL_PASS_DEPTH_FAIL", "i", GL_STENCIL_PASS_DEPTH_FAIL),
	GB_CONSTANT("GL_STENCIL_PASS_DEPTH_PASS", "i", GL_STENCIL_PASS_DEPTH_PASS),
	GB_CONSTANT("GL_STENCIL_REF", "i", GL_STENCIL_REF),
	GB_CONSTANT("GL_STENCIL_WRITEMASK", "i", GL_STENCIL_WRITEMASK),
	GB_CONSTANT("GL_STENCIL_INDEX", "i", GL_STENCIL_INDEX),
	GB_CONSTANT("GL_KEEP", "i", GL_KEEP),
	GB_CONSTANT("GL_REPLACE", "i", GL_REPLACE),
	GB_CONSTANT("GL_INCR", "i", GL_INCR),
	GB_CONSTANT("GL_DECR", "i", GL_DECR),

	/* Buffers, Pixel Drawing/Reading */
	GB_CONSTANT("GL_AUX0", "i", GL_AUX0),
	GB_CONSTANT("GL_AUX1", "i", GL_AUX1),
	GB_CONSTANT("GL_AUX2", "i", GL_AUX2),
	GB_CONSTANT("GL_AUX3", "i", GL_AUX3),
	GB_CONSTANT("GL_BACK_LEFT", "i", GL_BACK_LEFT),
	GB_CONSTANT("GL_BACK_RIGHT", "i", GL_BACK_RIGHT),
	GB_CONSTANT("GL_FRONT_LEFT", "i", GL_FRONT_LEFT),
	GB_CONSTANT("GL_FRONT_RIGHT", "i", GL_FRONT_RIGHT),
	GB_CONSTANT("GL_LEFT", "i", GL_LEFT),
	GB_CONSTANT("GL_NONE", "i", GL_NONE),
	GB_CONSTANT("GL_RIGHT", "i", GL_RIGHT),
	GB_CONSTANT("GL_COLOR_INDEX", "i", GL_COLOR_INDEX),
	GB_CONSTANT("GL_RED", "i", GL_RED),
	GB_CONSTANT("GL_GREEN", "i", GL_GREEN),
	GB_CONSTANT("GL_BLUE", "i", GL_BLUE),
	GB_CONSTANT("GL_ALPHA", "i", GL_ALPHA),
	GB_CONSTANT("GL_LUMINANCE", "i", GL_LUMINANCE),
	GB_CONSTANT("GL_LUMINANCE_ALPHA", "i", GL_LUMINANCE_ALPHA),
	GB_CONSTANT("GL_ALPHA_BITS", "i", GL_ALPHA_BITS),
	GB_CONSTANT("GL_RED_BITS", "i", GL_RED_BITS),
	GB_CONSTANT("GL_GREEN_BITS", "i", GL_GREEN_BITS),
	GB_CONSTANT("GL_BLUE_BITS", "i", GL_BLUE_BITS),
	GB_CONSTANT("GL_INDEX_BITS", "i", GL_INDEX_BITS),
	GB_CONSTANT("GL_SUBPIXEL_BITS", "i", GL_SUBPIXEL_BITS),
	GB_CONSTANT("GL_AUX_BUFFERS", "i", GL_AUX_BUFFERS),
	GB_CONSTANT("GL_READ_BUFFER", "i", GL_READ_BUFFER),
	GB_CONSTANT("GL_DRAW_BUFFER", "i", GL_DRAW_BUFFER),
	GB_CONSTANT("GL_DOUBLEBUFFER", "i", GL_DOUBLEBUFFER),
	GB_CONSTANT("GL_STEREO", "i", GL_STEREO),
	GB_CONSTANT("GL_BITMAP", "i", GL_BITMAP),
	GB_CONSTANT("GL_COLOR", "i", GL_COLOR),
	GB_CONSTANT("GL_DEPTH", "i", GL_DEPTH),
	GB_CONSTANT("GL_STENCIL", "i", GL_STENCIL),
	GB_CONSTANT("GL_DITHER", "i", GL_DITHER),
	GB_CONSTANT("GL_RGB", "i", GL_RGB),
	GB_CONSTANT("GL_RGBA", "i", GL_RGBA),

	/* Implementation limits */
	GB_CONSTANT("GL_MAX_LIST_NESTING", "i", GL_MAX_LIST_NESTING),
	GB_CONSTANT("GL_MAX_ATTRIB_STACK_DEPTH", "i", GL_MAX_ATTRIB_STACK_DEPTH),
	GB_CONSTANT("GL_MAX_NAME_STACK_DEPTH", "i", GL_MAX_NAME_STACK_DEPTH),
	GB_CONSTANT("GL_MAX_PROJECTION_STACK_DEPTH", "i", GL_MAX_PROJECTION_STACK_DEPTH),
	GB_CONSTANT("GL_MAX_TEXTURE_STACK_DEPTH", "i", GL_MAX_TEXTURE_STACK_DEPTH),
	GB_CONSTANT("GL_MAX_EVAL_ORDER", "i", GL_MAX_EVAL_ORDER),
	GB_CONSTANT("GL_MAX_LIGHTS", "i", GL_MAX_LIGHTS),
	GB_CONSTANT("GL_MAX_CLIP_PLANES", "i", GL_MAX_CLIP_PLANES),
	GB_CONSTANT("GL_MAX_TEXTURE_SIZE", "i", GL_MAX_TEXTURE_SIZE),
	GB_CONSTANT("GL_MAX_PIXEL_MAP_TABLE", "i", GL_MAX_PIXEL_MAP_TABLE),
	GB_CONSTANT("GL_MAX_VIEWPORT_DIMS", "i", GL_MAX_VIEWPORT_DIMS),
	GB_CONSTANT("GL_MAX_CLIENT_ATTRIB_STACK_DEPTH", "i", GL_MAX_CLIENT_ATTRIB_STACK_DEPTH),
	GB_CONSTANT("GL_MAX_MODELVIEW_STACK_DEPTH", "i", GL_MAX_MODELVIEW_STACK_DEPTH),

	/* Gets */
	GB_CONSTANT("GL_ATTRIB_STACK_DEPTH", "i", GL_ATTRIB_STACK_DEPTH),
	GB_CONSTANT("GL_CLIENT_ATTRIB_STACK_DEPTH", "i", GL_CLIENT_ATTRIB_STACK_DEPTH),
	GB_CONSTANT("GL_COLOR_CLEAR_VALUE", "i", GL_COLOR_CLEAR_VALUE),
	GB_CONSTANT("GL_COLOR_WRITEMASK", "i", GL_COLOR_WRITEMASK),
	GB_CONSTANT("GL_CURRENT_INDEX", "i", GL_CURRENT_INDEX),
	GB_CONSTANT("GL_CURRENT_COLOR", "i", GL_CURRENT_COLOR),
	GB_CONSTANT("GL_CURRENT_NORMAL", "i", GL_CURRENT_NORMAL),
	GB_CONSTANT("GL_CURRENT_RASTER_COLOR", "i", GL_CURRENT_RASTER_COLOR),
	GB_CONSTANT("GL_CURRENT_RASTER_DISTANCE", "i", GL_CURRENT_RASTER_DISTANCE),
	GB_CONSTANT("GL_CURRENT_RASTER_INDEX", "i", GL_CURRENT_RASTER_INDEX),
	GB_CONSTANT("GL_CURRENT_RASTER_POSITION", "i", GL_CURRENT_RASTER_POSITION),
	GB_CONSTANT("GL_CURRENT_RASTER_TEXTURE_COORDS", "i", GL_CURRENT_RASTER_TEXTURE_COORDS),
	GB_CONSTANT("GL_CURRENT_RASTER_POSITION_VALID", "i", GL_CURRENT_RASTER_POSITION_VALID),
	GB_CONSTANT("GL_CURRENT_TEXTURE_COORDS", "i", GL_CURRENT_TEXTURE_COORDS),
	GB_CONSTANT("GL_INDEX_CLEAR_VALUE", "i", GL_INDEX_CLEAR_VALUE),
	GB_CONSTANT("GL_INDEX_MODE", "i", GL_INDEX_MODE),
	GB_CONSTANT("GL_INDEX_WRITEMASK", "i", GL_INDEX_WRITEMASK),
	GB_CONSTANT("GL_MODELVIEW_MATRIX", "i", GL_MODELVIEW_MATRIX),
	GB_CONSTANT("GL_MODELVIEW_STACK_DEPTH", "i", GL_MODELVIEW_STACK_DEPTH),
	GB_CONSTANT("GL_NAME_STACK_DEPTH", "i", GL_NAME_STACK_DEPTH),
	GB_CONSTANT("GL_PROJECTION_MATRIX", "i", GL_PROJECTION_MATRIX),
	GB_CONSTANT("GL_PROJECTION_STACK_DEPTH", "i", GL_PROJECTION_STACK_DEPTH),
	GB_CONSTANT("GL_RENDER_MODE", "i", GL_RENDER_MODE),
	GB_CONSTANT("GL_RGBA_MODE", "i", GL_RGBA_MODE),
	GB_CONSTANT("GL_TEXTURE_MATRIX", "i", GL_TEXTURE_MATRIX),
	GB_CONSTANT("GL_TEXTURE_STACK_DEPTH", "i", GL_TEXTURE_STACK_DEPTH),
	GB_CONSTANT("GL_VIEWPORT", "i", GL_VIEWPORT),

	/* Evaluators */
	GB_CONSTANT("GL_AUTO_NORMAL", "i", GL_AUTO_NORMAL),
#if 0
/* Evaluators */
#define GL_MAP1_COLOR_4				0x0D90
#define GL_MAP1_INDEX				0x0D91
#define GL_MAP1_NORMAL				0x0D92
#define GL_MAP1_TEXTURE_COORD_1			0x0D93
#define GL_MAP1_TEXTURE_COORD_2			0x0D94
#define GL_MAP1_TEXTURE_COORD_3			0x0D95
#define GL_MAP1_TEXTURE_COORD_4			0x0D96
#define GL_MAP1_VERTEX_3			0x0D97
#define GL_MAP1_VERTEX_4			0x0D98
#define GL_MAP2_COLOR_4				0x0DB0
#define GL_MAP2_INDEX				0x0DB1
#define GL_MAP2_NORMAL				0x0DB2
#define GL_MAP2_TEXTURE_COORD_1			0x0DB3
#define GL_MAP2_TEXTURE_COORD_2			0x0DB4
#define GL_MAP2_TEXTURE_COORD_3			0x0DB5
#define GL_MAP2_TEXTURE_COORD_4			0x0DB6
#define GL_MAP2_VERTEX_3			0x0DB7
#define GL_MAP2_VERTEX_4			0x0DB8
#define GL_MAP1_GRID_DOMAIN			0x0DD0
#define GL_MAP1_GRID_SEGMENTS			0x0DD1
#define GL_MAP2_GRID_DOMAIN			0x0DD2
#define GL_MAP2_GRID_SEGMENTS			0x0DD3
#define GL_COEFF				0x0A00
#define GL_ORDER				0x0A01
#define GL_DOMAIN				0x0A02
#endif
	/* Hints */
	GB_CONSTANT("GL_FOG_HINT", "i", GL_FOG_HINT),
	GB_CONSTANT("GL_LINE_SMOOTH_HINT", "i", GL_LINE_SMOOTH_HINT),
	GB_CONSTANT("GL_PERSPECTIVE_CORRECTION_HINT", "i", GL_PERSPECTIVE_CORRECTION_HINT),
	GB_CONSTANT("GL_POINT_SMOOTH_HINT", "i", GL_POINT_SMOOTH_HINT),
	GB_CONSTANT("GL_POLYGON_SMOOTH_HINT", "i", GL_POLYGON_SMOOTH_HINT),
	GB_CONSTANT("GL_DONT_CARE", "i", GL_DONT_CARE),
	GB_CONSTANT("GL_FASTEST", "i", GL_FASTEST),
	GB_CONSTANT("GL_NICEST", "i", GL_NICEST),

	/* Scissor box */
	GB_CONSTANT("GL_SCISSOR_BOX", "i", GL_SCISSOR_BOX),
	GB_CONSTANT("GL_SCISSOR_TEST", "i", GL_SCISSOR_TEST),
#if 0
/* Pixel Mode / Transfer */
#define GL_MAP_COLOR				0x0D10
#define GL_MAP_STENCIL				0x0D11
#define GL_INDEX_SHIFT				0x0D12
#define GL_INDEX_OFFSET				0x0D13
#define GL_RED_SCALE				0x0D14
#define GL_RED_BIAS				0x0D15
#define GL_GREEN_SCALE				0x0D18
#define GL_GREEN_BIAS				0x0D19
#define GL_BLUE_SCALE				0x0D1A
#define GL_BLUE_BIAS				0x0D1B
#define GL_ALPHA_SCALE				0x0D1C
#define GL_ALPHA_BIAS				0x0D1D
#define GL_DEPTH_SCALE				0x0D1E
#define GL_DEPTH_BIAS				0x0D1F
#define GL_PIXEL_MAP_S_TO_S_SIZE		0x0CB1
#define GL_PIXEL_MAP_I_TO_I_SIZE		0x0CB0
#define GL_PIXEL_MAP_I_TO_R_SIZE		0x0CB2
#define GL_PIXEL_MAP_I_TO_G_SIZE		0x0CB3
#define GL_PIXEL_MAP_I_TO_B_SIZE		0x0CB4
#define GL_PIXEL_MAP_I_TO_A_SIZE		0x0CB5
#define GL_PIXEL_MAP_R_TO_R_SIZE		0x0CB6
#define GL_PIXEL_MAP_G_TO_G_SIZE		0x0CB7
#define GL_PIXEL_MAP_B_TO_B_SIZE		0x0CB8
#define GL_PIXEL_MAP_A_TO_A_SIZE		0x0CB9
#define GL_PIXEL_MAP_S_TO_S			0x0C71
#define GL_PIXEL_MAP_I_TO_I			0x0C70
#define GL_PIXEL_MAP_I_TO_R			0x0C72
#define GL_PIXEL_MAP_I_TO_G			0x0C73
#define GL_PIXEL_MAP_I_TO_B			0x0C74
#define GL_PIXEL_MAP_I_TO_A			0x0C75
#define GL_PIXEL_MAP_R_TO_R			0x0C76
#define GL_PIXEL_MAP_G_TO_G			0x0C77
#define GL_PIXEL_MAP_B_TO_B			0x0C78
#define GL_PIXEL_MAP_A_TO_A			0x0C79
#define GL_PACK_ALIGNMENT			0x0D05
#define GL_PACK_LSB_FIRST			0x0D01
#define GL_PACK_ROW_LENGTH			0x0D02
#define GL_PACK_SKIP_PIXELS			0x0D04
#define GL_PACK_SKIP_ROWS			0x0D03
#define GL_PACK_SWAP_BYTES			0x0D00
#define GL_UNPACK_ALIGNMENT			0x0CF5
#define GL_UNPACK_LSB_FIRST			0x0CF1
#define GL_UNPACK_ROW_LENGTH			0x0CF2
#define GL_UNPACK_SKIP_PIXELS			0x0CF4
#define GL_UNPACK_SKIP_ROWS			0x0CF3
#define GL_UNPACK_SWAP_BYTES			0x0CF0
#define GL_ZOOM_X				0x0D16
#define GL_ZOOM_Y				0x0D17
#endif
	/* Texture mapping */
	GB_CONSTANT("GL_TEXTURE_ENV", "i", GL_TEXTURE_ENV),
	GB_CONSTANT("GL_TEXTURE_ENV_MODE", "i", GL_TEXTURE_ENV_MODE),
	GB_CONSTANT("GL_TEXTURE_1D", "i", GL_TEXTURE_1D),
	GB_CONSTANT("GL_TEXTURE_2D", "i", GL_TEXTURE_2D),
	GB_CONSTANT("GL_TEXTURE_WRAP_S", "i", GL_TEXTURE_WRAP_S),
	GB_CONSTANT("GL_TEXTURE_WRAP_T", "i", GL_TEXTURE_WRAP_T),
	GB_CONSTANT("GL_TEXTURE_MAG_FILTER", "i", GL_TEXTURE_MAG_FILTER),
	GB_CONSTANT("GL_TEXTURE_MIN_FILTER", "i", GL_TEXTURE_MIN_FILTER),
	GB_CONSTANT("GL_TEXTURE_ENV_COLOR", "i", GL_TEXTURE_ENV_COLOR),
	GB_CONSTANT("GL_TEXTURE_GEN_S", "i", GL_TEXTURE_GEN_S),
	GB_CONSTANT("GL_TEXTURE_GEN_T", "i", GL_TEXTURE_GEN_T),
	GB_CONSTANT("GL_TEXTURE_GEN_MODE", "i", GL_TEXTURE_GEN_MODE),
	GB_CONSTANT("GL_TEXTURE_BORDER_COLOR", "i", GL_TEXTURE_BORDER_COLOR),
	GB_CONSTANT("GL_TEXTURE_WIDTH", "i", GL_TEXTURE_WIDTH),
	GB_CONSTANT("GL_TEXTURE_HEIGHT", "i", GL_TEXTURE_HEIGHT),
	GB_CONSTANT("GL_TEXTURE_BORDER", "i", GL_TEXTURE_BORDER),
	GB_CONSTANT("GL_TEXTURE_COMPONENTS", "i", GL_TEXTURE_COMPONENTS),
	GB_CONSTANT("GL_TEXTURE_RED_SIZE", "i", GL_TEXTURE_RED_SIZE),
	GB_CONSTANT("GL_TEXTURE_GREEN_SIZE", "i", GL_TEXTURE_GREEN_SIZE),
	GB_CONSTANT("GL_TEXTURE_BLUE_SIZE", "i", GL_TEXTURE_BLUE_SIZE),
	GB_CONSTANT("GL_TEXTURE_ALPHA_SIZE", "i", GL_TEXTURE_ALPHA_SIZE),
	GB_CONSTANT("GL_TEXTURE_LUMINANCE_SIZE", "i", GL_TEXTURE_LUMINANCE_SIZE),
	GB_CONSTANT("GL_TEXTURE_INTENSITY_SIZE", "i", GL_TEXTURE_INTENSITY_SIZE),
	GB_CONSTANT("GL_NEAREST_MIPMAP_NEAREST", "i", GL_NEAREST_MIPMAP_NEAREST),
	GB_CONSTANT("GL_NEAREST_MIPMAP_LINEAR", "i", GL_NEAREST_MIPMAP_LINEAR),
	GB_CONSTANT("GL_LINEAR_MIPMAP_NEAREST", "i", GL_LINEAR_MIPMAP_NEAREST),
	GB_CONSTANT("GL_LINEAR_MIPMAP_LINEAR", "i", GL_LINEAR_MIPMAP_LINEAR),
	GB_CONSTANT("GL_OBJECT_LINEAR", "i", GL_OBJECT_LINEAR),
	GB_CONSTANT("GL_OBJECT_PLANE", "i", GL_OBJECT_PLANE),
	GB_CONSTANT("GL_EYE_LINEAR", "i", GL_EYE_LINEAR),
	GB_CONSTANT("GL_EYE_PLANE", "i", GL_EYE_PLANE),
	GB_CONSTANT("GL_SPHERE_MAP", "i", GL_SPHERE_MAP),
	GB_CONSTANT("GL_DECAL", "i", GL_DECAL),
	GB_CONSTANT("GL_MODULATE", "i", GL_MODULATE),
	GB_CONSTANT("GL_NEAREST", "i", GL_NEAREST),
	GB_CONSTANT("GL_REPEAT", "i", GL_REPEAT),
	GB_CONSTANT("GL_CLAMP", "i", GL_CLAMP),
	GB_CONSTANT("GL_S", "i", GL_S),
	GB_CONSTANT("GL_T", "i", GL_T),
	GB_CONSTANT("GL_R", "i", GL_R),
	GB_CONSTANT("GL_Q", "i", GL_Q),
	GB_CONSTANT("GL_TEXTURE_GEN_R", "i", GL_TEXTURE_GEN_R),
	GB_CONSTANT("GL_TEXTURE_GEN_Q", "i", GL_TEXTURE_GEN_Q),


#if 0

/* OpenGL 1.1 */

  GB_CONSTANT("ProxyTexture1d", "i", GL_PROXY_TEXTURE_1D),
  GB_CONSTANT("ProxyTexture2d", "i", GL_PROXY_TEXTURE_2D),
  GB_CONSTANT("TexturePriority", "i", GL_TEXTURE_PRIORITY),
  GB_CONSTANT("TextureResident", "i", GL_TEXTURE_RESIDENT),
  GB_CONSTANT("TextureBinding1d", "i", GL_TEXTURE_BINDING_1D),
  GB_CONSTANT("TextureBinding2d", "i", GL_TEXTURE_BINDING_2D),
  GB_CONSTANT("TextureInternalFormat", "i", GL_TEXTURE_INTERNAL_FORMAT),
  GB_CONSTANT("Alpha4", "i", GL_ALPHA4),
  GB_CONSTANT("Alpha8", "i", GL_ALPHA8),
  GB_CONSTANT("Alpha12", "i", GL_ALPHA12),
  GB_CONSTANT("Alpha16", "i", GL_ALPHA16),
  GB_CONSTANT("Luminance4", "i", GL_LUMINANCE4),
  GB_CONSTANT("Luminance8", "i", GL_LUMINANCE8),
  GB_CONSTANT("Luminance12", "i", GL_LUMINANCE12),
  GB_CONSTANT("Luminance16", "i", GL_LUMINANCE16),
  GB_CONSTANT("Luminance4Alpha4", "i", GL_LUMINANCE4_ALPHA4),
  GB_CONSTANT("Luminance6Alpha2", "i", GL_LUMINANCE6_ALPHA2),
  GB_CONSTANT("Luminance8Alpha8", "i", GL_LUMINANCE8_ALPHA8),
  GB_CONSTANT("Luminance12Alpha4", "i", GL_LUMINANCE12_ALPHA4),
  GB_CONSTANT("Luminance12Alpha12", "i", GL_LUMINANCE12_ALPHA12),
  GB_CONSTANT("Luminance16Alpha16", "i", GL_LUMINANCE16_ALPHA16),
  GB_CONSTANT("Intensity", "i", GL_INTENSITY),
  GB_CONSTANT("Intensity4", "i", GL_INTENSITY4),
  GB_CONSTANT("Intensity8", "i", GL_INTENSITY8),
  GB_CONSTANT("Intensity12", "i", GL_INTENSITY12),
  GB_CONSTANT("Intensity16", "i", GL_INTENSITY16),
  GB_CONSTANT("R3G3B2", "i", GL_R3_G3_B2),
  GB_CONSTANT("Rgb4", "i", GL_RGB4),
  GB_CONSTANT("Rgb5", "i", GL_RGB5),
  GB_CONSTANT("Rgb8", "i", GL_RGB8),
  GB_CONSTANT("Rgb10", "i", GL_RGB10),
  GB_CONSTANT("Rgb12", "i", GL_RGB12),
  GB_CONSTANT("Rgb16", "i", GL_RGB16),
  GB_CONSTANT("Rgba2", "i", GL_RGBA2),
  GB_CONSTANT("Rgba4", "i", GL_RGBA4),
  GB_CONSTANT("Rgb5A1", "i", GL_RGB5_A1),
  GB_CONSTANT("Rgba8", "i", GL_RGBA8),
  GB_CONSTANT("Rgb10A2", "i", GL_RGB10_A2),
  GB_CONSTANT("Rgba12", "i", GL_RGBA12),
  GB_CONSTANT("Rgba16", "i", GL_RGBA16),
  GB_CONSTANT("ClientPixelStoreBit", "i", GL_CLIENT_PIXEL_STORE_BIT),
  GB_CONSTANT("ClientVertexArrayBit", "i", GL_CLIENT_VERTEX_ARRAY_BIT),
  GB_CONSTANT("AllClientAttribBits", "i", GL_ALL_CLIENT_ATTRIB_BITS),
  GB_CONSTANT("ClientAllAttribBits", "i", GL_CLIENT_ALL_ATTRIB_BITS),

/* texture_border_clamp */

  GB_CONSTANT("ClampToBorder", "i", GL_CLAMP_TO_BORDER),
#endif

	GB_END_DECLARE
};
