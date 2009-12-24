/***************************************************************************

  GL.c

  (c) 2009 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#include "GL.h"

#include "GLshader.h"
#include "GLprogram.h"
#include "GLuniform.h"

GB_DESC Cgl[] =
{
	GB_DECLARE("Gl",0), GB_NOT_CREATABLE(),
	
	/* GLshader.c */
	GB_STATIC_METHOD("AttachShader", NULL, GLATTACHSHADER, "(Program)i(Shader)i"),
	GB_STATIC_METHOD("CompileShader", NULL, GLCOMPILESHADER, "(Shader)i"),
	GB_STATIC_METHOD("CreateShader", "i", GLCREATESHADER, "(Type)i"),
	GB_STATIC_METHOD("DeleteShader", NULL, GLDELETESHADER, "(Shader)i"),
	GB_STATIC_METHOD("DetachShader", NULL, GLDETACHSHADER, "(Program)i(Shader)i"),
	GB_STATIC_METHOD("GetAttachedShader", "Integer[]", GLGETATTACHEDSHADERS, "(Program)i"),
	GB_STATIC_METHOD("GetShaderInfoLog", "s", GLGETSHADERINFOLOG, "(Shader)i"),
	GB_STATIC_METHOD("GetShaderiv", "Integer[]", GLGETSHADERIV, "(Shader)i(Pname)i"),
	GB_STATIC_METHOD("GetShaders", "Integer[]", GLGETATTACHEDSHADERS, "(Program)i"),
	GB_STATIC_METHOD("GetShaderSource", "s", GLGETSHADERSOURCE, "(Shader)i"),
	GB_STATIC_METHOD("IsShader", "b", GLISSHADER, "(Shader)i"),
	GB_STATIC_METHOD("ShaderSource", NULL, GLSHADERSOURCE, "(Shader)i(Source)s"),
	/* GLprogram.c */
	GB_STATIC_METHOD("CreateProgram", "i", GLCREATEPROGRAM, NULL),
	GB_STATIC_METHOD("DeleteProgram", NULL, GLDELETEPROGRAM, "(Program)i"),
	GB_STATIC_METHOD("GetProgramInfoLog", "s", GLGETPROGRAMINFOLOG, "(Program)i"),
	GB_STATIC_METHOD("GetProgramiv", "Integer[]", GLGETPROGRAMIV, "(Program)i(Pname)i"),
	GB_STATIC_METHOD("IsProgram", "b", GLISPROGRAM, "(Program)i"),
	GB_STATIC_METHOD("LinkProgram", NULL, GLLINKPROGRAM, "(Program)i"),
	GB_STATIC_METHOD("UseProgram", NULL, GLUSEPROGRAM, "(Program)i"),
	GB_STATIC_METHOD("ValidateProgram", NULL, GLVALIDATEPROGRAM, "(Program)i"),
	/* GLuniform.c */
	GB_STATIC_METHOD("Uniform1f", NULL, GLUNIFORM1F, "(Location)i(V0)f"),
	GB_STATIC_METHOD("Uniform2f", NULL, GLUNIFORM2F, "(Location)i(V0)f(V1)f"),
	GB_STATIC_METHOD("Uniform3f", NULL, GLUNIFORM3F, "(Location)i(V0)f(V1)f(V2)f"),
	GB_STATIC_METHOD("Uniform4f", NULL, GLUNIFORM4F, "(Location)i(V0)f(V1)f(V3)f(V3)f"),
	GB_STATIC_METHOD("Uniform1i", NULL, GLUNIFORM1I, "(Location)i(V0)i"),
	GB_STATIC_METHOD("Uniform2i", NULL, GLUNIFORM2I, "(Location)i(V0)i(V1)i"),
	GB_STATIC_METHOD("Uniform3i", NULL, GLUNIFORM3I, "(Location)i(V0)i(V1)i(V2)i"),
	GB_STATIC_METHOD("Uniform4i", NULL, GLUNIFORM4I, "(Location)i(V0)i(V1)i(V3)i(V3)i"),
//	GB_STATIC_METHOD("Uniform1fv", NULL, GLUNIFORMFV, "(Location)i(Values)Float[]"),
//	GB_STATIC_METHOD("Uniformiv", NULL, GLUNIFORMIV, "(Location)i(Values)Integer[]"),

	/* Contants */
	GB_CONSTANT("GL_ACTIVE_ATTRIBUTES", "i", GL_ACTIVE_ATTRIBUTES),
	GB_CONSTANT("GL_ACTIVE_ATTRIBUTE_MAX_LENGTH", "i", GL_ACTIVE_ATTRIBUTE_MAX_LENGTH),
	GB_CONSTANT("GL_ACTIVE_UNIFORMS", "i", GL_ACTIVE_UNIFORMS),
	GB_CONSTANT("GL_ACTIVE_UNIFORM_MAX_LENGTH", "i", GL_ACTIVE_UNIFORM_MAX_LENGTH),
	GB_CONSTANT("GL_ATTACHED_SHADERS", "i", GL_ATTACHED_SHADERS),
	GB_CONSTANT("GL_COMPILE_STATUS", "i", GL_COMPILE_STATUS),
	GB_CONSTANT("GL_CURRENT_PROGRAM", "i", GL_CURRENT_PROGRAM),
	GB_CONSTANT("GL_DELETE_STATUS", "i", GL_DELETE_STATUS),
	GB_CONSTANT("GL_FRAGMENT_SHADER", "i", GL_FRAGMENT_SHADER),
	GB_CONSTANT("GL_INFO_LOG_LENGTH", "i", GL_INFO_LOG_LENGTH),
	GB_CONSTANT("GL_LINK_STATUS", "i", GL_LINK_STATUS),
	GB_CONSTANT("GL_SHADER_SOURCE_LENGTH", "i", GL_SHADER_SOURCE_LENGTH),
	GB_CONSTANT("GL_SHADER_TYPE", "i", GL_SHADER_TYPE),
	GB_CONSTANT("GL_VALIDATE_STATUS", "i", GL_VALIDATE_STATUS),
	GB_CONSTANT("GL_VERTEX_SHADER", "i", GL_VERTEX_SHADER),
	
	GB_END_DECLARE
};
