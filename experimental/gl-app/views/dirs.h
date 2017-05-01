/**
* +----------------------------------------------------------------------+
* | This file is part of Samplecat. http://ayyi.github.io/samplecat/     |
* | copyright (C) 2016-2017 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __views_dirs_h__
#define __views_dirs_h__

typedef struct {
   AGlActor    actor;
   struct {
      int      n_rows;
      int      n_rows_visible;
   } cache;
} DirectoriesView;

AGlActor* directories_view (WaveformActor*);

#endif
