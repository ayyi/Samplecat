/**
* +----------------------------------------------------------------------+
* | This file is part of Samplecat. http://ayyi.github.io/samplecat/     |
* | copyright (C) 2015-2017 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __views_search_h__
#define __views_search_h__

typedef struct {
   AGlActor     actor;
   int          cursor_pos;
   char*        text;
   PangoLayout* layout;
   uint32_t     layout_colour;
} SearchView;

AGlActorClass* search_view_get_class ();

AGlActor* search_view        (gpointer);
int       search_view_height (SearchView*);

#endif
