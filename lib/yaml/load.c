/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://www.ayyi.org           |
* | copyright (C) 2007-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/

#include "config.h"
#include <stdint.h>
#include <glib.h>
#include "debug/debug.h"
#include "yaml/load.h"


/*
 *  Each top-level section in the yaml file is passed to its matching handler
 */
bool
yaml_load (FILE* fp, YamlHandler handlers[])
{
	yaml_parser_t parser; yaml_parser_initialize(&parser);

	yaml_parser_set_input_file(&parser, fp);

	int section = 0;

	// read the event sequence
	int safety = 0;
	char key[64] = "";
	bool end = false;
	yaml_event_t event;

	get_expected_event(&parser, &event, YAML_STREAM_START_EVENT);
	get_expected_event(&parser, &event, YAML_DOCUMENT_START_EVENT);

	do {
		if (!yaml_parser_parse(&parser, &event)) goto error; // Get the next event

		switch (event.type) {
			case YAML_STREAM_START_EVENT:
				dbg(2, "YAML_STREAM_START_EVENT");
				break;
			case YAML_STREAM_END_EVENT:
				end = TRUE;
				dbg(2, "YAML_STREAM_END_EVENT");
				break;
			case YAML_DOCUMENT_START_EVENT:
				dbg(2, "YAML_DOCUMENT_START_EVENT");
				break;
			case YAML_DOCUMENT_END_EVENT:
				end = TRUE;
				dbg(2, "YAML_DOCUMENT_END_EVENT");
				break;
			case YAML_ALIAS_EVENT:
				dbg(0, "YAML_ALIAS_EVENT");
				break;
			case YAML_SCALAR_EVENT:
				//dbg(0, "YAML_SCALAR_EVENT: value=%s %i plain=%i style=%i", event.data.scalar.value, event.data.scalar.length, event.data.scalar.plain_implicit, event.data.scalar.style);

				if(!key[0]){ // 1st half of a pair
					strncpy(key, (char*)event.data.scalar.value, 63);
				}else{
					// 2nd half of a pair
					dbg(2, "      %s=%s", key, event.data.scalar.value);
					key[0] = '\0';
				}
				break;
			case YAML_SEQUENCE_START_EVENT:
				dbg(2, "YAML_SEQUENCE_START_EVENT");
				break;
			case YAML_SEQUENCE_END_EVENT:
				dbg(2, "YAML_SEQUENCE_END_EVENT");
				break;
			case YAML_MAPPING_START_EVENT:
				if(key[0]){
					if(!section){
						dbg(2, "found section! %s", key);
						int i = 0;
						YamlHandler* h;
						while((h = &handlers[i]) && h->key ){
							if(!strcmp(h->key, key)){
								h->callback(&parser, &event, h->data);
								break;
							}
							i++;
						}
					}
					else dbg(2, "new section: %s", key);
					key[0] = '\0';
				}
				else dbg(2, "YAML_MAPPING_START_EVENT");
				break;
			case YAML_MAPPING_END_EVENT:
				dbg(2, "YAML_MAPPING_END_EVENT");
				break;
			case YAML_NO_EVENT:
				dbg(2, "YAML_NO_EVENT");
				break;
		}

		yaml_event_delete(&event);

	} while(!end && safety++ < 1024);

	yaml_parser_delete(&parser);
	fclose(fp);

	return true;

  error:
	yaml_parser_delete(&parser);
	fclose(fp);

	return false;
}


/*
 *  After having entered a new mapping, handle expected scalar and mapping events
 */
bool
load_mapping (yaml_parser_t* parser, yaml_event_t* event, YamlHandler scalars[], YamlMappingHandler mappings[], gpointer user_data)
{
	char key[64] = {0,};
	while(yaml_parser_parse(parser, event)){
		switch (event->type) {
			case YAML_SCALAR_EVENT:
				dbg(2, "YAML_SCALAR_EVENT: value='%s' %i plain=%i style=%i", event->data.scalar.value, event->data.scalar.length, event->data.scalar.plain_implicit, event->data.scalar.style);

				g_strlcpy(key, (char*)event->data.scalar.value, 64);

				int i = 0;
				YamlHandler* h;
				while((h = &scalars[i]) && h->key ){
					if(!strcmp(h->key, key)){
						h->callback(parser, event, h->data);
						key[0] = 0;
						break;
					}
					i++;
				}
				break;
			case YAML_MAPPING_START_EVENT:
				g_return_val_if_fail(key[0], false);
				if(key[0]){
					int i = 0;
					YamlMappingHandler* h;
					while((h = &mappings[i]) && h->key ){
						if(!strcmp(h->key, key)){
							h->callback(parser, event, key, h->data);
							break;
						}
						i++;
					}
				}
				break;
			case YAML_MAPPING_END_EVENT:
				return true;
			default:
				pwarn("unexpected event");
				return false;
		}
	}
	return false;
}


bool
handle_scalar_event (yaml_parser_t* parser, yaml_event_t* event, YamlHandler handlers[])
{
	char* key = (char*)event->data.scalar.value;
	int i = 0;
	YamlHandler* h;
	while((h = &handlers[i]) && h->callback){
		if(!h->key){
			h->callback(parser, event, h->data);
			break;
		}
		if(!strcmp(h->key, key)){
			h->callback(parser, event, h->data);
			break;
		}
		i++;
	}

	return true;
}


bool
find_event (yaml_parser_t* parser, yaml_event_t* event, const char* name)
{
	bool found = false;
	while(!found && yaml_parser_parse(parser, event)){
		switch (event->type) {
			case YAML_SCALAR_EVENT:
				if(!strcmp((char*)event->data.scalar.value, name)){
					found = true;
				}
				break;
			case YAML_NO_EVENT:
				return false;
			default:
				break;
		}
	}
	return found;
}


void
yaml_set_string (yaml_event_t* event, gpointer data)
{
	g_return_if_fail(event->type == YAML_SCALAR_EVENT);
	*((char**)data) = g_strdup((char*)event->data.scalar.value);
}


void
yaml_set_int (yaml_event_t* event, gpointer data)
{
	g_return_if_fail(event->type == YAML_SCALAR_EVENT);
	*((int*)data) = atoi((char*)event->data.scalar.value);
}


void
yaml_set_uint64 (yaml_event_t* event, gpointer data)
{
	g_return_if_fail(event->type == YAML_SCALAR_EVENT);
	*((uint64_t*)data) = strtoull((char*)(event->data.scalar.value), NULL, 10);
}

