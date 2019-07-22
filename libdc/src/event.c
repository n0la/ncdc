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

#include <dc/event.h>
#include "internal.h"

struct dc_event_
{
    dc_refable_t ref;

    char *type;
    json_t *payload;
};

static void dc_event_free(dc_event_t e)
{
    return_if_true(e == NULL,);

    free(e->type);
    json_decref(e->payload);

    free(e);
}

dc_event_t dc_event_new(char const *type, json_t *payload)
{
    return_if_true(type == NULL, NULL);

    dc_event_t e = calloc(1, sizeof(struct dc_event_));
    return_if_true(e == NULL, NULL);

    e->ref.cleanup = (dc_cleanup_t)dc_event_free;

    /* A long, long time ago I had a rather quirky software engineering
     * professor. He taught us C, and everytime string handling would be
     * the topic we would wait for one scene: He'd put his index finger on
     * his chin, barely touching his lower lip, raise both eyebrows in
     * astonishment, and wonder, and then he'd say "strdup" in the worst
     * German accent. Even after 15 years that scene stuck with me.
     */
    e->type = strdup(type);

    if (payload != NULL) {
        e->payload = json_incref(payload);
    } else {
        e->payload = json_null();
    }

    return dc_ref(e);
}

char const *dc_event_type(dc_event_t e)
{
    return_if_true(e == NULL, NULL);
    return e->type;
}

json_t *dc_event_payload(dc_event_t e)
{
    return_if_true(e == NULL, NULL);
    return e->payload;
}

dc_event_type_t dc_event_type_code(dc_event_t e)
{
    static char const *types[DC_EVENT_TYPE_LAST] = {
        [DC_EVENT_TYPE_UNKNOWN] = "UNKNOWN",
        [DC_EVENT_TYPE_READY] = "READY",
        [DC_EVENT_TYPE_MESSAGE_CREATE] = "MESSAGE_CREATE",
    };

    int i = 0;

    for (i = 0; i < DC_EVENT_TYPE_LAST; i++) {
        if (strcmp(types[i], e->type) == 0) {
            return (dc_event_type_t)i;
        }
    }

    return DC_EVENT_TYPE_UNKNOWN;
}
