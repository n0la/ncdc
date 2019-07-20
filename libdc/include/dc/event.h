/*
 * Part of ncdc - a discord client for the console
 * Copyright (C) 2019 Florian Stinglmayr <fstinglmayr@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DC_EVENT_H
#define DC_EVENT_H

#include <stdint.h>
#include <jansson.h>

struct dc_event_;
typedef struct dc_event_ *dc_event_t;

typedef enum {
    DC_EVENT_TYPE_UNKNOWN = 0,
    DC_EVENT_TYPE_READY,
    DC_EVENT_TYPE_MESSAGE_CREATE,

    /* ^^^^^^ Make sure events are up there ^^^^^^^ */
    DC_EVENT_TYPE_LAST,
} dc_event_type_t;

dc_event_t dc_event_new(char const *type, json_t *payload);

/**
 * Returns the event type. This is an upper case string, with
 * words separated by underscores. For a list of available event
 * types please see the Discord documentation.
 */
char const *dc_event_type(dc_event_t e);

/**
 * The JSON payload associated with the given event type. Note this
 * could be json_null() if the event has no associated payload.
 */
json_t *dc_event_payload(dc_event_t e);

/**
 * Returns an integer code representing the given type string.
 */
dc_event_type_t dc_event_type_code(dc_event_t e);

#endif
