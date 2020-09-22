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

#include "c_webview.h"

#define HISTORY (webkit_web_view_get_back_forward_list(WIDGET))

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

BEGIN_PROPERTY(WebView_Title)

	GB.ReturnNewZeroString(webkit_web_view_get_title(WIDGET));

END_PROPERTY

BEGIN_METHOD(WebView_SetHtml, GB_STRING html; GB_STRING root)

	webkit_web_view_load_html(WIDGET, GB.ToZeroString(ARG(html)), MISSING(root) ? NULL : GB.ToZeroString(ARG(root)));

END_METHOD

BEGIN_METHOD_VOID(WebView_Back)

	webkit_web_view_go_back(WIDGET);

END_METHOD

BEGIN_METHOD_VOID(WebView_Forward)

	webkit_web_view_go_forward(WIDGET);

END_METHOD

BEGIN_METHOD(WebView_Reload, GB_BOOLEAN bypass)

	if (VARGOPT(bypass, FALSE))
		webkit_web_view_reload_bypass_cache(WIDGET);
	else
		webkit_web_view_reload(WIDGET);

END_METHOD

BEGIN_METHOD_VOID(WebView_Stop)

	webkit_web_view_stop_loading(WIDGET);

END_METHOD

BEGIN_PROPERTY(WebView_Zoom)

	if (READ_PROPERTY)
		GB.ReturnFloat(webkit_web_view_get_zoom_level(WIDGET));
	else
		webkit_web_view_set_zoom_level(WIDGET, VPROP(GB_FLOAT));

END_PROPERTY

//---------------------------------------------------------------------------

BEGIN_PROPERTY(WebViewHistoryItem_Title)

	GB.ReturnNewZeroString(webkit_back_forward_list_item_get_title(THIS->item));

END_PROPERTY

BEGIN_PROPERTY(WebViewHistoryItem_Url)

	GB.ReturnNewZeroString(webkit_back_forward_list_item_get_uri(THIS->item));

END_PROPERTY

BEGIN_METHOD_VOID(WebViewHistoryItem_GoTo)

	webkit_web_view_go_to_back_forward_list_item(WIDGET, THIS->item);

END_METHOD

BEGIN_METHOD_VOID(WebViewHistory_Clear)

	fprintf(stderr, "gb.gtk3.webview: warning: WebView.History.Clear() does nothing yet.\n");

END_METHOD

BEGIN_METHOD(WebViewHistory_get, GB_INTEGER index)

	WebKitBackForwardListItem *item = webkit_back_forward_list_get_nth_item(HISTORY, VARG(index));
	
	if (!item)
		GB.ReturnNull();
	else
	{
		THIS->item = item;
		RETURN_SELF();
	}

END_METHOD

BEGIN_PROPERTY(WebViewHistory_CanGoBack)

	GB.ReturnBoolean(webkit_web_view_can_go_forward(WIDGET));

END_PROPERTY

BEGIN_PROPERTY(WebViewHistory_CanGoForward)

	GB.ReturnBoolean(webkit_web_view_can_go_forward(WIDGET));

END_PROPERTY

//---------------------------------------------------------------------------

GB_DESC WebViewHistoryItemDesc[] = 
{
	GB_DECLARE_VIRTUAL(".WebView.History.Item"),
	
	GB_PROPERTY_READ("Title", "s", WebViewHistoryItem_Title),
	GB_PROPERTY_READ("Url", "s", WebViewHistoryItem_Url),
	GB_METHOD("GoTo", NULL, WebViewHistoryItem_GoTo, NULL),
	
	GB_END_DECLARE
};

GB_DESC WebViewHistoryDesc[] =
{
	GB_DECLARE_VIRTUAL(".WebView.History"),

	GB_METHOD("Clear", NULL, WebViewHistory_Clear, NULL),
	GB_METHOD("_get", ".WebView.History.Item", WebViewHistory_get, "(Index)i"),
	GB_PROPERTY_READ("CanGoBack", "b", WebViewHistory_CanGoBack),
	GB_PROPERTY_READ("CanGoForward", "b", WebViewHistory_CanGoForward),

	GB_END_DECLARE
};

GB_DESC WebViewDesc[] =
{
  GB_DECLARE("WebView", sizeof(CWEBVIEW)), GB_INHERITS("Control"),

  //GB_STATIC_METHOD("_exit", NULL, GLArea_exit, NULL),

  GB_METHOD("_new", NULL, WebView_new, "(Parent)Container;"),
  //GB_METHOD("_free", NULL, GLArea_free, NULL),
  
  GB_PROPERTY("Url", "s", WebView_Url),
	GB_PROPERTY("Title", "s", WebView_Title),
	GB_PROPERTY("Zoom", "f", WebView_Zoom),

	GB_METHOD("SetHtml", NULL, WebView_SetHtml, "(Html)s[(Root)s]"),

	GB_METHOD("Back", NULL, WebView_Back, NULL),
	GB_METHOD("Forward", NULL, WebView_Forward, NULL),
	GB_METHOD("Reload", NULL, WebView_Reload, "[(BypassCache)b]"),
	GB_METHOD("Stop", NULL, WebView_Stop, NULL),
	
	GB_PROPERTY_SELF("History", ".WebView.History"),

	GB_CONSTANT("_Properties", "s", "*,Url,Zoom=1"),
  GB_CONSTANT("_Group", "s", "View"),

  //GB_EVENT("Open", NULL, NULL, &EVENT_Open),
  //GB_EVENT("Draw", NULL, NULL, &EVENT_Draw),
  //GB_EVENT("Resize", NULL, NULL, &EVENT_Resize),

  GB_END_DECLARE
};
