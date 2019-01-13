#ifndef __have_sample_h__
#define __have_sample_h__

#include <stdlib.h>
#include <stdint.h>
#ifdef USE_GTK
#include <gtk/gtk.h>
#else
#include <gdk/gdk.h>
#endif
#include "typedefs.h"

struct _Sample
{
	int          id;            // database index or -1 for external file.
	int          ref_count;

	char*        name;          ///< basename UTF8
	char*        sample_dir;    ///< directory UTF8
	char*        full_path;     ///< filename as native to system

	unsigned int sample_rate;
	int64_t      length;        // milliseconds
	int64_t      frames;        // total number of frames (eg a frame for 16bit stereo is 4 bytes).
	unsigned int channels;
	int          bit_depth;
	int          bit_rate;
	GPtrArray*   meta_data;
	float        peaklevel;
	int          colour_index;

	gboolean     online;
	time_t       mtime;

	char*        keywords;
	char*        ebur;
	char*        notes;
	char*        mimetype;

	GdkPixbuf*   overview;

	void*        row_ref;
};


/** create new sample structures
 * all _new, _dup functions imply sample_ref() 
 *
 * @return needs to be sample_unref();
 */
Sample*     sample_new               ();
Sample*     sample_new_from_filename (char* path, gboolean path_alloced);
Sample*     sample_dup               (Sample*);

Sample*     sample_get_by_filename   (const char* abspath);

Sample*     sample_ref               (Sample*);
void        sample_unref             (Sample*);

bool        sample_get_file_info     (Sample*);

char*       sample_get_metadata_str  (Sample*);
void        sample_set_metadata      (Sample*, const char*);

#define sample_unref0(var) ((var == NULL) ? NULL : (var = (sample_unref (var), NULL)))

#endif
