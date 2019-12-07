/**
* +----------------------------------------------------------------------+
* | This file is part of Samplecat. http://ayyi.github.io/samplecat/     |
* | copyright (C) 2016-2019 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __state_h__
#define __state_h__

#include "agl/behaviour.h"

typedef union
{
    int         i;
    char*       c;
} AMVal;

typedef struct
{
    char*   name;
    int     utype;
    union {
        void (*i)(AGlActor*, int);
        void (*f)(AGlActor*, float);
        void (*c)(AGlActor*, const char*);
    }         set;
    AMVal val;
    AMVal min;
    AMVal max;
    AMVal dfault;

} ConfigParam;

typedef struct
{
    int         size;
    ConfigParam params[];
} ParamArray;

typedef struct {
    AGlBehaviour  behaviour;
    ParamArray*   params;
} StateBehaviour;

AGlBehaviourClass* state_get_class           ();
AGlBehaviour*      state                     ();
bool               state_set_named_parameter (AGlActor*, char* name, char* value);

#endif