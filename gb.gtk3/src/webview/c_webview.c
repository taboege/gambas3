/***************************************************************************

  c_webview.c

  (c) Benoît Minisini <g4mba5@gmail.com>

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

#define __C_WEBVIEW_C

#include <webkit2/webkit2.h>
#include "c_webview.h"

//---------------------------------------------------------------------------

//DECLARE_EVENT(EVENT_Open);

//---------------------------------------------------------------------------

BEGIN_METHOD(WebView_new, GB_OBJECT parent)

	THIS->widget = webkit_web_view_new();

	GTK.CreateControl(THIS, VARG(parent), THIS->widget);
	
	//webkit_web_view_load_uri(WIDGET, "http://google.fr/");	
	//g_signal_connect(G_OBJECT(THIS->widget), "configure-event", G_CALLBACK(cb_reshape_ext), (gpointer)THIS);
	//g_signal_connect(G_OBJECT(THIS->widget), "expose-event", G_CALLBACK(cb_draw_ext), (gpointer)THIS);
	
END_METHOD

BEGIN_PROPERTY(WebView_Url)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(webkit_web_view_get_uri(WIDGET));
	else
		webkit_web_view_load_uri(WIDGET, GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY

//---------------------------------------------------------------------------

GB_DESC WebViewDesc[] =
{
  GB_DECLARE("WebView", sizeof(CWEBVIEW)), GB_INHERITS("Control"),

  //GB_STATIC_METHOD("_exit", NULL, GLArea_exit, NULL),

  GB_METHOD("_new", NULL, WebView_new, "(Parent)Container;"),
  //GB_METHOD("_free", NULL, GLArea_free, NULL),
  //GB_METHOD("Update", NULL, GLArea_update, NULL),
  //GB_METHOD("Refresh", NULL, GLArea_Refresh, NULL),
  //GB_METHOD("Select", NULL, CGLAREA_select, NULL),
  //GB_METHOD("Text", NULL, CGLAREA_text, "(Text)s(X)i(Y)i"),
  
  GB_PROPERTY("Url", "s", WebView_Url),

  GB_CONSTANT("_Properties", "s", "*,Url"),
  GB_CONSTANT("_Group", "s", "View"),

  //GB_EVENT("Open", NULL, NULL, &EVENT_Open),
  //GB_EVENT("Draw", NULL, NULL, &EVENT_Draw),
  //GB_EVENT("Resize", NULL, NULL, &EVENT_Resize),

  GB_END_DECLARE
};
