/**
* +----------------------------------------------------------------------+
* | This file is part of Samplecat. http://ayyi.github.io/samplecat/     |
* | copyright (C) 2012-2016 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#define __wf_private__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>
#include <waveform/ass.h>
#include "agl/ext.h"
#include "agl/utils.h"
#include "agl/actor.h"
#include "waveform/waveform.h"
#include "waveform/peakgen.h"
#include "waveform/shader.h"
#include "waveform/actors/text.h"
#include "samplecat.h"
#include "views/list.h"

extern int need_draw;

#define _g_free0(var) (var = (g_free (var), NULL))

#define FONT "Droid Sans"

static AGl* agl = NULL;
static int instance_count = 0;


static void
_init()
{
	static bool init_done = false;

	if(!init_done){
		agl = agl_get_instance();
		agl_set_font_string("Roboto 10"); // initialise the pango context

		init_done = true;
	}
}


AGlActor*
list_view(WaveformActor* _)
{
	instance_count++;

	_init();

	bool list_paint(AGlActor* actor)
	{
		ListView* view = (ListView*)actor;

		#define row_height 20
		#define N_ROWS_VISIBLE(A) (agl_actor__height(((AGlActor*)A)) / row_height)
		int n_rows = N_ROWS_VISIBLE(actor);

		int col[] = {0, 150, 260, 360, 420};

		GtkTreeIter iter;
		if(!gtk_tree_model_get_iter_first((GtkTreeModel*)samplecat.store, &iter)){ gerr ("cannot get iter."); return false; }
		int i = 0;
		for(;i<view->scroll_offset;i++){
			gtk_tree_model_iter_next((GtkTreeModel*)samplecat.store, &iter);
		}
		int row_count = 0;
		do {
			if(row_count == view->selection - view->scroll_offset){
				agl->shaders.plain->uniform.colour = 0x6677ff77;
				agl_use_program((AGlShader*)agl->shaders.plain);
				agl_rect_((AGlRect){0, row_count * row_height - 2, agl_actor__width(actor), row_height});
			}

			Sample* sample = samplecat_list_store_get_sample_by_iter(&iter);
			if(sample){
				char* len[32]; format_smpte((char*)len, sample->frames);
				char* f[32]; samplerate_format((char*)f, sample->sample_rate);

				char* val[4] = {sample->name, sample->sample_dir, (char*)len, (char*)f};

				int c; for(c=0;c<G_N_ELEMENTS(val);c++){
					agl_enable_stencil(0, 0, col[c + 1] - 6, actor->region.y2);
					agl_print(col[c], row_count * row_height, 0, 0xffffffff, val[c]);
				}
				sample_unref(sample);
			}
		} while (++row_count < n_rows && gtk_tree_model_iter_next((GtkTreeModel*)samplecat.store, &iter));

		agl_disable_stencil();

		return true;
	}

	void list_init(AGlActor* a)
	{
#ifdef AGL_ACTOR_RENDER_CACHE
		a->fbo = agl_fbo_new(agl_actor__width(a), agl_actor__height(a), 0, AGL_FBO_HAS_STENCIL);
		a->cache.enabled = true;
#endif
	}

	void list_set_size(AGlActor* actor)
	{
	}

	bool list_event(AGlActor* actor, GdkEvent* event, AGliPt xy)
	{
									// TODO why is y not relative to actor.y ?
		switch(event->type){
			case GDK_BUTTON_PRESS:
			case GDK_BUTTON_RELEASE:
				agl_actor__invalidate(actor);
				int row = (xy.y - actor->region.y1) / row_height;
				dbg(0, "y=%i", xy.y - actor->region.y1);
				list_view_select((ListView*)actor, row);
				break;
			default:
				break;
		}
		return AGL_HANDLED;
	}

	void list_free(AGlActor* actor)
	{
		if(!--instance_count){
		}
	}

	ListView* view = g_new0(ListView, 1);
	AGlActor* actor = (AGlActor*)view;
#ifdef AGL_DEBUG_ACTOR
	actor->name = "List";
#endif
	actor->init = list_init;
	actor->free = list_free;
	actor->paint = list_paint;
	actor->set_size = list_set_size;
	actor->on_event = list_event;

	void on_selection_change(SamplecatModel* m, Sample* sample, gpointer user_data)
	{
		PF;
	}
	g_signal_connect((gpointer)samplecat.model, "selection-changed", G_CALLBACK(on_selection_change), NULL);

	void list_on_search_filter_changed(GObject* _filter, gpointer _actor)
	{
		//             update list...
		agl_actor__invalidate((AGlActor*)_actor);
	}
	g_signal_connect(samplecat.model->filters.search, "changed", G_CALLBACK(list_on_search_filter_changed), actor);

	return actor;
}


void
list_view_select(ListView* list, int row)
{
	int n_rows_total = ((SamplecatListStore*)samplecat.store)->row_count;

	if(row >= 0 && row < n_rows_total){
		list->selection = row;
		if(list->selection >= list->scroll_offset + N_ROWS_VISIBLE(list)){
			dbg(0, "need to scroll down");
			list->scroll_offset++;
		}else if(list->selection < list->scroll_offset){
			list->scroll_offset--;
		}
		agl_actor__invalidate((AGlActor*)list);

		Sample* sample = samplecat_list_store_get_sample_by_row_index(list->selection);
		if(sample){
			samplecat_model_set_selection (samplecat.model, sample);
			sample_unref(sample);
		}
	}
}

