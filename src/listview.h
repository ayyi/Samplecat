/**
* +----------------------------------------------------------------------+
* | This file is part of Samplecat. http://ayyi.github.io/samplecat/     |
* | copyright (C) 2007-2019 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __listview_h__
#define __listview_h__

#include "list_store.h"

struct _libraryview {
   GtkWidget*         widget;         // treeview
   GtkWidget*         scroll;

   struct {
      GtkCellRenderer* name;
      GtkCellRenderer* tags;
   }                   cells;
   GtkTreeViewColumn* col_name;
   GtkTreeViewColumn* col_path;
   GtkTreeViewColumn* col_pixbuf;
   GtkTreeViewColumn* col_tags;

   int                  selected;
   GtkTreeRowReference* mouseover_row_ref;
};

GtkWidget*  listview__new                  ();
void        listview__show_db_missing      ();
void        listview__add_item             ();

void        listview__reset_colours        ();
void        listview__edit_row             (GtkWidget*, gpointer);

void        listview__block_motion_handler ();
gint        listview__get_mouseover_row    ();


#endif
