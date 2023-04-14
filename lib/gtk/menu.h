/*
 +----------------------------------------------------------------------+
 | This file is part of Samplecat. http://ayyi.github.io/samplecat/     |
 | copyright (C) 2007-2023 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#pragma once

#include <gtk/gtk.h>

typedef struct
{
	char*     label;
	GCallback callback;
	char*     stock_id;
	int       callback_data;
} MenuDef;

GtkWidget* make_menu                (int size, MenuDef[size], gpointer);
void       add_menu_items_from_defn (GtkWidget* menu, GMenuModel*, int size, MenuDef[size], gpointer);
