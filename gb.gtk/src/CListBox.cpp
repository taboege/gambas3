/***************************************************************************

  CListBox.cpp

  (c) 2004-2005 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#define __CLISTBOX_CPP

#include "gambas.h"
#include "main.h"
#include "widgets.h"

#include "CListBox.h"
#include "CWidget.h"
#include "CContainer.h"

#include <stdlib.h>

#include <stdio.h>

DECLARE_EVENT(EVENT_Select);    /* selection change */
DECLARE_EVENT(EVENT_Click);     /* simple click */
DECLARE_EVENT(EVENT_Activate);  /* double click */

static void raise_select(gTreeView *sender)
{
	CWIDGET *_ob = GetObject((gControl*)sender);
	
	if (!_ob) return;
	GB.Raise((void*)_ob, EVENT_Select, 0);
}

static void raise_activate(gTreeView *sender, char *key)
{
	CWIDGET *_ob = GetObject((gControl*)sender);
	
	if (!_ob) return;
	GB.Raise((void*)_ob, EVENT_Activate, 0);
}

static void raise_click(gTreeView *sender)
{
	CWIDGET *_ob = GetObject((gControl*)sender);
	
	if (!_ob) return;
	GB.Raise((void*)_ob, EVENT_Click, 0);
}


BEGIN_METHOD(CLISTBOX_new, GB_OBJECT parent)

	InitControl(new gListBox(CONTAINER(VARG(parent))), (CWIDGET*)THIS);
	
	LISTBOX->onSelect = raise_select;
	LISTBOX->onActivate = raise_activate;
	LISTBOX->onClick = raise_click;

END_METHOD


BEGIN_METHOD_VOID(CLISTBOX_clear)

	LISTBOX->clear();

END_METHOD


BEGIN_METHOD(CLISTBOX_get, GB_INTEGER index)

	long index = VARG(index);

	if (index < 0 || index >= LISTBOX->count())
	{
		GB.Error("Bad index");
		return;
	}

	THIS->index = index;
	RETURN_SELF();

END_METHOD



BEGIN_METHOD(CLISTBOX_add, GB_STRING item; GB_INTEGER pos)

	LISTBOX->add( GB.ToZeroString(ARG(item)), VARGOPT(pos, -1));	

END_METHOD



BEGIN_METHOD(CLISTBOX_remove, GB_INTEGER pos)

	LISTBOX->remove(VARG(pos));

END_METHOD


BEGIN_PROPERTY(CLISTBOX_sorted)

	if (READ_PROPERTY) { GB.ReturnBoolean(LISTBOX->isSorted()); return; }
	LISTBOX->setSorted(VPROP(GB_BOOLEAN));

END_METHOD


BEGIN_PROPERTY(CLISTBOX_count)

	GB.ReturnInteger(LISTBOX->count());

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_index)

	if (READ_PROPERTY) { GB.ReturnInteger(LISTBOX->index()); return; }
	LISTBOX->setIndex(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_current)

	THIS->index=LISTBOX->index();
	if (THIS->index>=0) RETURN_SELF();
	else GB.ReturnNull();

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_text)

	GB.ReturnNewZeroString(LISTBOX->text());

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_mode)

	if (READ_PROPERTY)
		GB.ReturnInteger(LISTBOX->mode());
	else
		LISTBOX->setMode(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_item_selected)

	if (READ_PROPERTY)
		GB.ReturnBoolean(LISTBOX->isItemSelected(THIS->index));
	else
		LISTBOX->setItemSelected(THIS->index,VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CLISTBOX_item_text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(LISTBOX->itemText(THIS->index));
	else	
		LISTBOX->setItemText(THIS->index,GB.ToZeroString(PROP(GB_STRING)));

END_PROPERTY


BEGIN_METHOD(CLISTBOX_find, GB_STRING item)

	GB.ReturnInteger(LISTBOX->find(GB.ToZeroString(ARG(item))));

END_METHOD


BEGIN_PROPERTY(CLISTBOX_list)

	GB_ARRAY array;
	int i;
	
	if (READ_PROPERTY)
	{
		GB.Array.New(&array, GB_T_STRING, LISTBOX->count());
		for (i = 0; i < LISTBOX->count(); i++)
		{
			*((char **)GB.Array.Get(array, i)) = GB.NewZeroString(LISTBOX->itemText(i));
		}
		
		GB.ReturnObject(array);
	}
	else
	{
		array = VPROP(GB_OBJECT);
		LISTBOX->lock();
		LISTBOX->clear();
		if (array)
		{
			for (i = 0; i < GB.Array.Count(array); i++)
				LISTBOX->add(*((char **)GB.Array.Get(array, i)));
		}
		LISTBOX->unlock();
	}

END_PROPERTY

BEGIN_PROPERTY(CLISTBOX_border)

	if (READ_PROPERTY)
		GB.ReturnBoolean(LISTBOX->hasBorder());
	else
		LISTBOX->setBorder(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_METHOD_VOID(ListBox_SelectAll)

	LISTBOX->selectAll();

END_METHOD

BEGIN_METHOD_VOID(ListBox_UnselectAll)

	LISTBOX->unselectAll();

END_METHOD

/***************************************************************************

  ListBoxItem

***************************************************************************/

GB_DESC CListBoxItemDesc[] =
{
  GB_DECLARE(".ListBox.Item", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Text", "s", CLISTBOX_item_text),
  GB_PROPERTY("Selected", "b", CLISTBOX_item_selected),

  GB_END_DECLARE
};

/***************************************************************************

  ListBox

***************************************************************************/

GB_DESC CListBoxDesc[] =
{
  GB_DECLARE("ListBox", sizeof(CLISTBOX)), GB_INHERITS("Control"),

  GB_METHOD("_new", 0, CLISTBOX_new, "(Parent)Container;"),
  GB_METHOD("_get", ".ListBox.Item", CLISTBOX_get, "(Index)i"),
  GB_METHOD("Clear", 0, CLISTBOX_clear, 0),
  GB_METHOD("Add", 0, CLISTBOX_add, "(Text)s[(Index)i]"),
  GB_METHOD("Remove", 0, CLISTBOX_remove, "(Index)i"),
  GB_METHOD("Find", "i", CLISTBOX_find, "(Item)s"),

  GB_PROPERTY("Sorted", "b", CLISTBOX_sorted),
  GB_PROPERTY("List", "String[]", CLISTBOX_list),
  GB_PROPERTY_READ("Count", "i", CLISTBOX_count),
  GB_PROPERTY_READ("Current", ".ListBox.Item", CLISTBOX_current),
  GB_PROPERTY_READ("Text", "s", CLISTBOX_text),
  GB_PROPERTY("Index", "i", CLISTBOX_index),
  GB_PROPERTY("Mode", "i", CLISTBOX_mode),
  GB_PROPERTY("Border", "b", CLISTBOX_border),
	GB_METHOD("SelectAll", NULL, ListBox_SelectAll, NULL),
	GB_METHOD("UnselectAll", NULL, ListBox_UnselectAll, NULL),

  GB_EVENT("Select", 0, 0, &EVENT_Select),
  GB_EVENT("Activate", 0, 0, &EVENT_Activate),
  GB_EVENT("Click", 0, 0, &EVENT_Click),

  LISTBOX_DESCRIPTION,

  GB_END_DECLARE
};



