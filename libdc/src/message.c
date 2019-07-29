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

#include <dc/message.h>
#include "internal.h"

struct dc_message_
{
    dc_refable_t ref;

    char *id;
    char *timestamp;
    char *content;
    char *channel_id;

    time_t ts;

    dc_account_t author;
};

static void dc_message_parse_timestamp(dc_message_t m);

static void dc_message_free(dc_message_t m)
{
    return_if_true(m == NULL,);

    free(m->id);
    free(m->timestamp);
    free(m->content);
    free(m->channel_id);

    dc_unref(m->author);

    free(m);
}

dc_message_t dc_message_new(void)
{
    dc_message_t m = calloc(1, sizeof(struct dc_message_));
    return_if_true(m == NULL, NULL);

    m->ref.cleanup = (dc_cleanup_t)dc_message_free;
    return dc_ref(m);
}

dc_message_t dc_message_new_content(char const *s, int len)
{
    dc_message_t m = dc_message_new();
    return_if_true(m == NULL, NULL);

    if (len < 0) {
        len = strlen(s);
    }

    m->content = strndup(s, len);
    if (m->content == NULL) {
        dc_unref(m);
        return NULL;
    }

    return m;
}

dc_message_t dc_message_from_json(json_t *j)
{
    dc_message_t m = NULL;
    json_t *val = NULL;
    return_if_true(j == NULL || !json_is_object(j), NULL);

    m = dc_message_new();
    return_if_true(m == NULL, NULL);

    val = json_object_get(j, "id");
    goto_if_true(val == NULL || !json_is_string(val), error);
    m->id = strdup(json_string_value(val));

    val = json_object_get(j, "timestamp");
    goto_if_true(val == NULL || !json_is_string(val), error);
    m->timestamp = strdup(json_string_value(val));

    dc_message_parse_timestamp(m);

    val = json_object_get(j, "content");
    goto_if_true(val == NULL || !json_is_string(val), error);
    m->content = strdup(json_string_value(val));

    val = json_object_get(j, "channel_id");
    goto_if_true(val == NULL || !json_is_string(val), error);
    m->channel_id = strdup(json_string_value(val));

    val = json_object_get(j, "author");
    goto_if_true(val == NULL || !json_is_object(val), error);
    m->author = dc_account_from_json(val);

    return m;

error:

    dc_unref(m);
    return NULL;
}

json_t *dc_message_to_json(dc_message_t m)
{
    return_if_true(m == NULL, NULL);

    json_t *j = json_object();
    return_if_true(j == NULL, NULL);

    if (m->id != NULL) {
        json_object_set_new(j, "id", json_string(m->id));
    }

    if (m->timestamp != NULL) {
        json_object_set_new(j, "timestamp", json_string(m->timestamp));
    }

    if (m->channel_id != NULL) {
        json_object_set_new(j, "channel_id", json_string(m->channel_id));
    }

    if (m->author != NULL) {
        json_t *a = dc_account_to_json(m->author);
        json_object_set_new(j, "author", a);
    }

    json_object_set_new(j, "content", json_string(m->content));

    return j;
}

static void dc_message_parse_timestamp(dc_message_t m)
{
    return_if_true(m == NULL || m->timestamp == NULL,);
    struct tm t = {0};

    strptime(m->timestamp, "%Y-%m-%dT%H:%M:%S", &t);
    m->ts = timegm(&t);
}

time_t dc_message_unix_timestamp(dc_message_t m)
{
    return_if_true(m == NULL, 0);
    return m->ts;
}

int dc_message_compare(dc_message_t *a, dc_message_t *b)
{
    return_if_true(a == NULL || *a == NULL ||
                   b == NULL || *b == NULL, 0);
    return (*a)->ts - (*b)->ts;
}

char const *dc_message_id(dc_message_t m)
{
    return_if_true(m == NULL, NULL);
    return m->id;
}

char const *dc_message_channel_id(dc_message_t m)
{
    return_if_true(m == NULL, NULL);
    return m->channel_id;
}

char const *dc_message_timestamp(dc_message_t m)
{
    return_if_true(m == NULL, NULL);
    return m->timestamp;
}

char const *dc_message_content(dc_message_t m)
{
    return_if_true(m == NULL, NULL);
    return m->content;
}

dc_account_t dc_message_author(dc_message_t m)
{
    return_if_true(m == NULL, NULL);
    return m->author;
}
