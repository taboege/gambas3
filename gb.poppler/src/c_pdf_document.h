/***************************************************************************

  c_pdf_document.h

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

#ifndef __C_PDF_DOCUMENT_H
#define __C_PDF_DOCUMENT_H

#include "poppler/cpp/poppler-document.h"
#include "poppler/cpp/poppler-page.h"
#include "poppler/cpp/poppler-page-renderer.h"
#include "poppler/cpp/poppler-image.h"
#include "main.h"

#ifndef __C_PDF_DOCUMENT_CPP
extern GB_DESC PdfDocumentDesc[];
extern GB_DESC PdfPageDesc[];
#endif

typedef
	struct {
		GB_BASE ob;
		poppler::document *doc;
		poppler::page **pages;
		poppler::page *current;
		poppler::page_renderer *renderer;
		double resolution;
		int rotation;
	}
	CPDFDOCUMENT;
	
#define THIS ((CPDFDOCUMENT *)_object)
	
#endif /* __C_PDF_DOCUMENT_H */
