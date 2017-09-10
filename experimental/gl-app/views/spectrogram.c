/**
* +----------------------------------------------------------------------+
* | This file is part of Samplecat. http://ayyi.github.io/samplecat/     |
* | copyright (C) 2017-2017 Tim Orford <tim@orford.org>                  |
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
#include <X11/keysym.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "debug/debug.h"
#include "agl/actor.h"
#include "agl/fbo.h"
#include "samplecat.h"
#include "audio_analysis/spectrogram/spectrogram.h"
#include "views/spectrogram.h"

#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

#define PADDING 1
#define BORDER 1

static AGl* agl = NULL;
static int instance_count = 0;
static AGlActorClass actor_class = {0, "Spectrogram", (AGlActorNew*)spectrogram_view};

static bool load_texture(SpectrogramView*);


AGlActorClass*
spectrogram_view_get_class ()
{
	return &actor_class;
}


static void
_init()
{
	static bool init_done = false;

	if(!init_done){
		agl = agl_get_instance();

		init_done = true;
	}
}

AGlActor*
spectrogram_view(gpointer _)
{
	instance_count++;

	_init();

	bool spectrogram_paint(AGlActor* actor)
	{
		SpectrogramView* view = (SpectrogramView*)actor;

		if(view->need_texture_change){
			load_texture(view);
			view->need_texture_change = false;
		}

		if(view->textures[0]){
			agl_textured_rect(view->textures[0], 0, 0, agl_actor__width(actor), agl_actor__height(actor), NULL);
		}

		return true;
	}

	void spectrogram_init(AGlActor* actor)
	{
		SpectrogramView* view = (SpectrogramView*)actor;

		glGenTextures(1, view->textures);
	}

	void spectrogram_size(AGlActor* actor)
	{
	}

	bool spectrogram_event(AGlActor* actor, GdkEvent* event, AGliPt xy)
	{
		return AGL_NOT_HANDLED;
	}

	void spectrogram_free(AGlActor* actor)
	{
		if(!--instance_count){
		}
	}

	SpectrogramView* view = AGL_NEW(SpectrogramView,
		.actor = {
			.class = &actor_class,
			.name = actor_class.name,
			.init = spectrogram_init,
			.free = spectrogram_free,
			.paint = spectrogram_paint,
			.set_size = spectrogram_size,
			.on_event = spectrogram_event,
		}
	);

	void spectrogram_on_selection_change(SamplecatModel* m, Sample* sample, gpointer actor)
	{
		SpectrogramView* view = (SpectrogramView*)actor;

		dbg(1, "sample=%s", sample->name);

		if(view->sample) sample_unref(view->sample);
		view->sample = sample_ref(sample);

		cancel_spectrogram(NULL);

		void spectrogram_ready(const char* filename, GdkPixbuf* pixbuf, gpointer _view)
		{
			PF;
			SpectrogramView* view = (SpectrogramView*)_view;

			if(pixbuf){
				if(view->pixbuf) g_object_unref(view->pixbuf);
				view->pixbuf = pixbuf;
				view->need_texture_change = true;

				agl_actor__invalidate((AGlActor*)view);
			}
		}
		get_spectrogram(sample->full_path, spectrogram_ready, view);
	}
	g_signal_connect((gpointer)samplecat.model, "selection-changed", G_CALLBACK(spectrogram_on_selection_change), view);

	AGlActor* actor = (AGlActor*)view;
	return actor;
}


static bool
load_texture(SpectrogramView* view)
{
	glBindTexture(GL_TEXTURE_2D, view->textures[0]);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLint n_colour_components = (GLint)gdk_pixbuf_get_n_channels(view->pixbuf);
	GLenum format = gdk_pixbuf_get_n_channels(view->pixbuf) == 4 ? GL_RGBA : GL_RGB;
	if(gluBuild2DMipmaps(GL_TEXTURE_2D, n_colour_components, (GLsizei)gdk_pixbuf_get_width(view->pixbuf), (GLsizei)gdk_pixbuf_get_height(view->pixbuf), format, GL_UNSIGNED_BYTE, gdk_pixbuf_get_pixels(view->pixbuf))){
		gwarn("mipmap generation failed");
		return false;
	}
	return true;
}