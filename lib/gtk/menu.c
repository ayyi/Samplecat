/*
 +----------------------------------------------------------------------+
 | This file is part of Samplecat. https://ayyi.github.io/samplecat/    |
 | copyright (C) 2007-2023 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#include "config.h"
#include "gtk/menu.h"
#include "debug/debug.h"


#ifdef GTK4_TODO
static void
menu_item_image_from_stock (GtkWidget* menu, GtkWidget* item, char* stock_id)
{
	GtkWidget* icon = gtk_image_new_from_icon_name(stock_id);

	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), icon);
}
#endif


GtkWidget*
make_menu (int size, MenuDef menu_def[size], gpointer user_data)
{
	GMenuModel* model = (GMenuModel*)g_menu_new ();
	GtkWidget* menu = gtk_popover_menu_new_from_model_full (model, GTK_POPOVER_MENU_NESTED);

	add_menu_items_from_defn (menu, model, size, menu_def, user_data);

	return menu;
}


void
add_menu_items_from_defn (GtkWidget* widget, GMenuModel* model, int size, MenuDef defn[size], gpointer user_data)
{
	GMenu* menu = G_MENU (model);

#ifdef GTK4_TODO
	GtkWidget* parent = widget;
	GtkWidget* sub = NULL;
#endif

	for (int i=0;i<size;i++) {
		MenuDef* item = &defn[i];
		switch (item->label[0]) {
			case '-':
				break;
#ifdef GTK4_TODO
			case '<':
				gtk_menu_item_set_submenu(GTK_MENU_ITEM(sub), parent);
				parent = widget;
				break;
			case '>':
				item = &defn[++i]; // following item must be the submenu parent item.
				parent = gtk_menu_new();

				GtkWidget* _sub = sub = gtk_image_menu_item_new_with_label(item->label);
				if(item->stock_id){
					menu_item_image_from_stock(widget, _sub, item->stock_id);
				}
				gtk_container_add(GTK_CONTAINER(widget), _sub);

				break;
#endif
			default:
				g_menu_append (menu, item->label, "todo-action");
#ifdef GTK4_TODO
				if (item->stock_id)
					menu_item_image_from_stock(widget, menu_item, item->stock_id);
#endif
#ifdef GTK4_TODO
				if (item->callback)
					g_signal_connect (G_OBJECT(menu_item), "activate", G_CALLBACK(item->callback), item->callback_data ? GINT_TO_POINTER(item->callback_data) : user_data);
#endif
		}
	}

	gtk_widget_show (widget);
}

