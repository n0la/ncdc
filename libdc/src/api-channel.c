#include <dc/api.h>

#include "internal.h"

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
