/***************************************************************************

  c_pdf_document.cpp

  gb.poppler component

  (c) Benoît Minisini <g4mba5@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __C_PDF_DOCUMENT_CPP

#include "poppler/cpp/poppler-global.h"
#include "c_pdf_document.h"

static void return_ustring(poppler::ustring str)
{
	poppler::byte_array utf8 = str.to_utf8();
	int len = utf8.size();
	if (len >= 1)
		GB.ReturnNewString(utf8.data(), len);
	else
		GB.ReturnVoidString();
}

static poppler::rotation_enum conv_rotation(int angle)
{
	if (angle < 0)
		angle = 360 - (-angle) % 360;
	else
		angle = angle % 360;
	
	switch (angle)
	{
		case 0: return poppler::rotate_0;
		case 90: return poppler::rotate_90;
		case 180: return poppler::rotate_180;
		case 270: return poppler::rotate_270;
		default: return poppler::rotate_0;
	}
}

//--------------------------------------------------------------------------

static void free_image(GB_IMG *img, void *image)
{
	delete (poppler::image *)image;
}

static void *temp_image(GB_IMG *img)
{
	poppler::image *image;

	if (!img->data)
		image = new poppler::image(0, 0, poppler::image::format_argb32);
	else
		image = new poppler::image((char *)img->data, img->width, img->height, poppler::image::format_argb32);
	
	return image;
}

static GB_IMG_OWNER _image_owner = {
	"gb.poppler",
	GB_IMAGE_BGRA,
	free_image,
	free_image,
	temp_image,
	NULL,
	};

static void return_image(poppler::image *image)
{
	GB_IMG *img;
	static GB_CLASS class_id = 0;

	if (!class_id)
		class_id = GB.FindClass("Image");

	img = (GB_IMG *)GB.New(class_id, NULL, NULL);
	
	IMAGE.Take(img, &_image_owner, image, image->width(), image->height(), (uchar *)image->const_data());	
	
	GB.ReturnObject(img);
}
	
//--------------------------------------------------------------------------

BEGIN_METHOD(PdfDocument_new, GB_STRING path; GB_STRING owner; GB_STRING password)

	char *buf;
	int len;
	std::string owner;
	std::string password;

	if (GB.LoadFile(STRING(path), LENGTH(path), &buf, &len))
		return;

	if (!MISSING(owner))
		owner = std::string(STRING(owner), LENGTH(owner));
	
	if (!MISSING(password))
		password = std::string(STRING(password), LENGTH(password));
	
	THIS->doc = poppler::document::load_from_raw_data(buf, len, owner, password);
	THIS->renderer = new poppler::page_renderer;
	THIS->resolution = 300.0;

END_METHOD

BEGIN_METHOD_VOID(PdfDocument_free)

	int i;
	
	if (THIS->pages)
	{
		for (i = 0; i < THIS->doc->pages(); i++)
			delete THIS->pages[i];
		
		GB.Free(POINTER(&THIS->pages));
	}

	delete THIS->renderer;
	delete THIS->doc;
	
END_METHOD

BEGIN_PROPERTY(PdfDocument_Count)

	GB.ReturnInteger(THIS->doc->pages());

END_PROPERTY

#define IMPLEMENT_DOC_PROP(_name, _func) \
BEGIN_PROPERTY(PdfDocument_##_name) \
	return_ustring(THIS->doc->_func()); \
END_PROPERTY

IMPLEMENT_DOC_PROP(Author, get_author)
IMPLEMENT_DOC_PROP(Creator, get_creator)
IMPLEMENT_DOC_PROP(Producer, get_producer)
IMPLEMENT_DOC_PROP(Subject, get_subject)
IMPLEMENT_DOC_PROP(Title, get_title)

BEGIN_METHOD(PdfDocument_get, GB_INTEGER index)

	int index = VARG(index);
	
	if (index < 0 || index >= THIS->doc->pages())
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}

	if (!THIS->pages)
		GB.AllocZero(POINTER(&THIS->pages), sizeof(void *) * THIS->doc->pages());
	
	if (!THIS->pages[index])
		THIS->pages[index] = THIS->doc->create_page(index);
	
	THIS->current = THIS->pages[index];
	
	RETURN_SELF();

END_METHOD

BEGIN_PROPERTY(PdfDocument_Resolution)

	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->resolution);
	else
	{
		double res = VPROP(GB_FLOAT);
		
		if (res <= 0)
		{
			GB.Error(GB_ERR_ARG);
			return;
		}
		THIS->resolution = res;
	}

END_PROPERTY

BEGIN_PROPERTY(PdfDocument_Rotation)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->rotation);
	else
		THIS->rotation = VPROP(GB_INTEGER);

END_PROPERTY

#define IMPLEMENT_HINT_PROP(_name, _const) \
BEGIN_PROPERTY(PdfDocument_##_name) \
	if (READ_PROPERTY) \
		GB.ReturnBoolean(THIS->renderer->render_hints() & poppler::page_renderer::_const); \
	else \
		THIS->renderer->set_render_hint(poppler::page_renderer::_const, VPROP(GB_BOOLEAN)); \
END_PROPERTY

IMPLEMENT_HINT_PROP(Antialiasing, antialiasing)
IMPLEMENT_HINT_PROP(TextAntialiasing, text_antialiasing)
IMPLEMENT_HINT_PROP(TextHinting, text_hinting)

//--------------------------------------------------------------------------

BEGIN_PROPERTY(PdfPage_Orientation)

	GB.ReturnInteger(THIS->current->orientation());

END_PROPERTY

BEGIN_METHOD(PdfPage_Render, GB_INTEGER x; GB_INTEGER y; GB_INTEGER width; GB_INTEGER height; GB_INTEGER rotation; GB_FLOAT res)

	int rotation = VARGOPT(rotation, THIS->rotation);
	double res = VARGOPT(res, THIS->resolution);
	int x = VARGOPT(x, -1);
	int y = VARGOPT(y, -1);
	int w = VARGOPT(width, -1);
	int h = VARGOPT(height, -1);
	
	poppler::image *image = new poppler::image;

	*image = THIS->renderer->render_page(THIS->current, res, res, x, y, w, h, conv_rotation(rotation));
	
	return_image(image);

END_METHOD

//--------------------------------------------------------------------------

GB_DESC PdfPageDesc[] =
{
	GB_DECLARE_VIRTUAL(".PdfPage"),
	
	GB_PROPERTY_READ("Orientation", "i", PdfPage_Orientation),
	
	GB_METHOD("Render", "Image", PdfPage_Render, "[(X)i(Y)i(Width)i(Height)i(Rotation)i(Resolution)f]"),
	
	GB_END_DECLARE
};

GB_DESC PdfDocumentDesc[] =
{
	GB_DECLARE("PdfDocument", sizeof(CPDFDOCUMENT)),

	/*GB_CONSTANT("Unknown","i",actionUnknown),
	GB_CONSTANT("Goto","i",actionGoTo),
	GB_CONSTANT("GotoRemote","i",actionGoToR),
	GB_CONSTANT("Launch","i",actionLaunch),
	GB_CONSTANT("Uri","i",actionURI),
	GB_CONSTANT("Named","i",actionNamed),
	GB_CONSTANT("Movie","i",actionMovie),

	GB_CONSTANT("Normal","i",0),
	GB_CONSTANT("Sideways","i",90),
	GB_CONSTANT("Inverted","i",180),
	GB_CONSTANT("SidewaysInverted","i",270),*/

	GB_METHOD("_new", 0, PdfDocument_new, "(Path)s[(Owner)s(Password)s]"),
	GB_METHOD("_free", 0, PdfDocument_free, 0),

	GB_METHOD("_get", ".PdfPage", PdfDocument_get, "(Index)i"),

	//GB_PROPERTY("Zoom", "f", PDFDOCUMENT_scale),
	//GB_PROPERTY("Orientation", "i", PDFDOCUMENT_rotation),

	//GB_PROPERTY_READ("Ready","b",PDFDOCUMENT_ready),
	GB_PROPERTY_READ("Count","i",PdfDocument_Count),
	//GB_PROPERTY_READ("HasIndex","b",PDFDOCUMENT_has_index),
	//GB_PROPERTY_READ("Index",".PdfDocument.Index",PDFDOCUMENT_index),
	//GB_PROPERTY_READ("Info",".PdfDocument.Info",PDFDOCUMENT_info),
	
	GB_PROPERTY_READ("Author", "s", PdfDocument_Author),
	GB_PROPERTY_READ("Creator", "s", PdfDocument_Creator),
	GB_PROPERTY_READ("Producer", "s", PdfDocument_Producer),
	GB_PROPERTY_READ("Subject", "s", PdfDocument_Subject),
	GB_PROPERTY_READ("Title", "s", PdfDocument_Title),
	
	GB_PROPERTY("Resolution", "f", PdfDocument_Resolution),
	GB_PROPERTY("Rotation", "i", PdfDocument_Rotation),
	GB_PROPERTY("Antialiasing", "b", PdfDocument_Antialiasing),
	GB_PROPERTY("TextAntialiasing", "b", PdfDocument_TextAntialiasing),
	GB_PROPERTY("TextHinting", "b", PdfDocument_TextHinting),
	
	GB_END_DECLARE
};

GB_DESC PdfDesc[] = 
{
	GB_DECLARE_STATIC("Pdf"),
	
	GB_CONSTANT("Landscape", "i", poppler::page::landscape),
	GB_CONSTANT("Portrait", "i", poppler::page::portrait),
	GB_CONSTANT("Seascape", "i", poppler::page::seascape),
	GB_CONSTANT("UpsideDown", "i", poppler::page::upside_down),

	GB_END_DECLARE
};
