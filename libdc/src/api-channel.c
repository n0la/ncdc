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

#include <dc/api.h>

#include "internal.h"

bool dc_api_channel_ack(dc_api_t api, dc_account_t login,
                        dc_channel_t c, dc_message_t m)
{
    bool ret = false;
    char *url = NULL;
    json_t *reply = NULL, *j = NULL;

    return_if_true(api == NULL || login == NULL ||
                   c == NULL || m == NULL, false);

    asprintf(&url, "channels/%s/messages/%s/ack",
             dc_channel_id(c),
             dc_message_id(m)
        );
    goto_if_true(url == NULL, cleanup);

    j = json_object();
    goto_if_true(j == NULL, cleanup);
    json_object_set_new(j, "token", json_string(TOKEN(login)));

    reply = dc_api_call_sync(api, "POST", TOKEN(login), url, j);
    goto_if_true(reply != NULL, cleanup);

    ret = true;

cleanup:

    free(url);
    json_decref(reply);
    json_decref(j);

    return ret;
}

bool dc_api_post_message(dc_api_t api, dc_account_t login,
                         dc_channel_t c, dc_message_t m)
{
    bool ret = false;
    char *url = NULL;
    json_t *j = NULL, *reply = NULL;

    return_if_true(api == NULL || login == NULL || m == NULL, false);
    return_if_true(dc_message_content(m) == NULL, false);

    asprintf(&url, "channels/%s/messages", dc_channel_id(c));
    goto_if_true(url == NULL, cleanup);

    j = dc_message_to_json(m);
    goto_if_true(j == NULL, cleanup);

    reply = dc_api_call_sync(api, "POST", TOKEN(login), url, j);
    goto_if_true(reply != NULL, cleanup);

    ret = true;

cleanup:

    free(url);
    json_decref(j);
    json_decref(reply);

    return ret;
}

bool dc_api_get_messages(dc_api_t api, dc_account_t login, dc_channel_t c)
{
    bool ret = false;
    char *url = NULL;
    json_t *reply = NULL, *i = NULL;
    GPtrArray *msgs = NULL;
    size_t idx = 0;

    return_if_true(api == NULL || login == NULL || c == NULL, false);

    msgs = g_ptr_array_new_with_free_func((GDestroyNotify)dc_unref);
    goto_if_true(msgs == NULL, cleanup);

    asprintf(&url, "channels/%s/messages", dc_channel_id(c));
    goto_if_true(url == NULL, cleanup);

    reply = dc_api_call_sync(api, "GET", TOKEN(login), url, NULL);
    goto_if_true(reply == NULL, cleanup);
    goto_if_true(!json_is_array(reply), cleanup);

    json_array_foreach(reply, idx, i) {
        dc_message_t m = dc_message_from_json(i);
        g_ptr_array_add(msgs, m);
    }

    dc_channel_add_messages(c, (dc_message_t*)msgs->pdata, msgs->len);
    ret = true;

cleanup:

    if (msgs != NULL) {
        g_ptr_array_unref(msgs);
        msgs = NULL;
    }

    json_decref(reply);
    free(url);

    return ret;
}

bool dc_api_create_channel(dc_api_t api, dc_account_t login,
                           dc_account_t *recipients, size_t nrecp,
                           dc_channel_t *channel)
{
    bool ret = false;
    json_t *data = NULL, *array = NULL, *reply = NULL;
    char *url = NULL;
    dc_channel_t c = NULL;
    size_t i = 0;

    return_if_true(api == NULL || login == NULL || channel == NULL, false);

    asprintf(&url, "users/%s/channels", dc_account_id(login));
    goto_if_true(url == NULL, cleanup);

    /* build a JSON object that contains one array called "recipients":
     * {"recipients": ["snowflake#1", ..., "snowflake#N"]}
     */
    data = json_object();
    array = json_array();
    goto_if_true(data == NULL || array == NULL, cleanup);

    for (i = 0; i < nrecp; i++) {
        dc_account_t r = recipients[0];
        if (dc_account_id(r) == NULL) {
            continue;
        }
        json_array_append_new(array, json_string(dc_account_id(r)));
    }

    goto_if_true(json_array_size(array) == 0, cleanup);
    json_object_set_new(data, "recipients", array);

    reply = dc_api_call_sync(api, "POST", dc_account_token(login), url, data);
    goto_if_true(reply == NULL, cleanup);

    c = dc_channel_from_json(reply);
    goto_if_true(c == NULL, cleanup);

    *channel = c;
    ret = true;

cleanup:

    free(url);
    json_decref(reply);
    json_decref(data);

    return ret;
}
