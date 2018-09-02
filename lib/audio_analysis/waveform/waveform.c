/**
* +----------------------------------------------------------------------+
* | This file is part of Samplecat. http://ayyi.github.io/samplecat/     |
* | copyright (C) 2007-2018 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixdata.h>
#include <cairo.h>
#include "debug/debug.h"
#include "waveform/waveform/waveform.h"
#include "decoder/ad.h"
#include "samplecat/support.h"
#include "samplecat/sample.h"

#define OVERVIEW_WIDTH (200)
#define OVERVIEW_HEIGHT (20)

static struct {
	GdkColor bg;
	GdkColor base;
	GdkColor text;
} colour = {
	{0,},
	{0,},
	{.red=0xdddd, .green=0xdddd, .blue=0xdddd},
};


static void
draw_cairo_line(cairo_t* cr, DRect* pts, double line_width, GdkColor* colour)
{
	float r, g, b;
	colour_get_float(colour, &r, &g, &b, 0xff);
	cairo_set_source_rgba (cr, b, g, r, 1.0);
	cairo_move_to (cr, pts->x1 - 0.5, pts->y1 +.5);
	cairo_line_to (cr, pts->x2 - 0.5, pts->y2 -.5);
	cairo_stroke (cr);
	cairo_move_to (cr, pts->x1 + 0.5, pts->y1 +.5);
	cairo_line_to (cr, pts->x2 + 0.5, pts->y2 -.5);
	cairo_stroke (cr);
}


static void
pixbuf_clear(GdkPixbuf* pixbuf, GdkColor* colour)
{
	guint32 colour_rgba = ((colour->red/256)<< 24) | ((colour->green/256)<<16) | ((colour->blue/256)<<8) | (0x60);
	gdk_pixbuf_fill(pixbuf, colour_rgba);
}


void
set_overview_colour (GdkColor* text_colour, GdkColor* base_colour, GdkColor* bg_colour)
{
	colour.bg = *bg_colour;
	colour.base = *base_colour;
	colour.text = *text_colour;
}


GdkPixbuf*
make_overview(Sample* sample)
{
	WfDecoder d = {{0,}};
	if(!ad_open(&d, sample->full_path)) return NULL;
	dbg(1, "NEW OVERVIEW");

	if(!strcmp(sample->mimetype, "audio/mp4")){
		// Using libwaveform for thumbnail generation is the way to go.
		// It is being tested for mp4 files as AAC is broken in the older path.
		GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, HAS_ALPHA_FALSE, BITS_PER_CHAR_8, OVERVIEW_WIDTH, OVERVIEW_HEIGHT);
		Waveform* waveform = waveform_load_new(sample->full_path);
		waveform_peak_to_pixbuf(waveform, pixbuf, NULL, color_gdk_to_rgba(&colour.text), color_gdk_to_rgba(&colour.base), true);
		g_object_unref(waveform);
		return sample->overview = pixbuf;
	}

	GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, HAS_ALPHA_TRUE, BITS_PER_CHAR_8, OVERVIEW_WIDTH, OVERVIEW_HEIGHT);
	pixbuf_clear(pixbuf, &colour.bg);

	cairo_format_t format;
	if (gdk_pixbuf_get_n_channels(pixbuf) == 3) format = CAIRO_FORMAT_RGB24; else format = CAIRO_FORMAT_ARGB32;
	cairo_surface_t* surface = cairo_image_surface_create_for_data (gdk_pixbuf_get_pixels(pixbuf), format, OVERVIEW_WIDTH, OVERVIEW_HEIGHT, gdk_pixbuf_get_rowstride(pixbuf));
	cairo_t* cr = cairo_create(surface);
	cairo_set_line_width (cr, 0.5);

	if (1) {
		DRect pts = {0, OVERVIEW_HEIGHT/2, OVERVIEW_WIDTH, OVERVIEW_HEIGHT/2 + 1};
		draw_cairo_line(cr, &pts, 1.0, &colour.base);
	}
	cairo_set_line_width (cr, 1.0);

	int frames_per_buf = d.info.frames / OVERVIEW_WIDTH;
	int buffer_len = frames_per_buf * d.info.channels;
	float* data = g_malloc(sizeof(float) * buffer_len);

	int x = 0;
	float min;                //negative peak value for each pixel.
	float max;                //positive peak value for each pixel.
	int readcount;
	while((readcount = ad_read(&d, data, buffer_len)) > 0){
		int frame;
		const int srcidx_start = 0;
		const int srcidx_stop  = frames_per_buf;

		min = 1.0; max = -1.0;
		for(frame=srcidx_start;frame<srcidx_stop;frame++){ 
			int ch;
			for(ch=0;ch<d.info.channels;ch++){
				if(frame * d.info.channels + ch > buffer_len){ perr("index error!\n"); break; }
				const float sample_val = data[frame * d.info.channels + ch];
				max = MAX(max, sample_val);
				min = MIN(min, sample_val);
			}
		}

		//scale the values to the view height:
		min = rint(min * OVERVIEW_HEIGHT/2.0);
		max = rint(max * OVERVIEW_HEIGHT/2.0);

		DRect pts = {x, OVERVIEW_HEIGHT/2 - min, x, OVERVIEW_HEIGHT/2 - max};
		draw_cairo_line(cr, &pts, 1.0, &colour.text);

		x++;
	}

	if(ad_close(&d)) perr("bad file close.\n");
	g_free(data);
	ad_free_nfo(&d.info);
	cairo_destroy(cr);
	cairo_surface_destroy(surface);
	sample->overview = pixbuf;
	if(!GDK_IS_PIXBUF(sample->overview)) perr("pixbuf is not a pixbuf.\n");
	return pixbuf;
}

