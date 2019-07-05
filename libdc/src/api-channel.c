#include <dc/api.h>

#include "internal.h"

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
     * {"recipients": ["snowflake#1", "snowflake#N"]}
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

cleanup:

    free(url);
    json_decref(reply);
    json_decref(data);

    return ret;
}
