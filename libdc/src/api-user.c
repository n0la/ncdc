#include <dc/api.h>
#include "internal.h"

bool dc_api_get_userinfo(dc_api_t api, dc_account_t login,
                         dc_account_t user)
{
    char *url = NULL;
    json_t *reply = NULL, *val = NULL;
    bool ret = false;

    return_if_true(api == NULL, false);
    return_if_true(login == NULL, false);
    return_if_true(user == NULL, false);

    if (user == login) {
        url = strdup("users/@me");
    } else {
        asprintf(&url, "users/%s", dc_account_id(user));
    }

    reply = dc_api_call_sync(api, "GET", dc_account_token(login), url, NULL);
    goto_if_true(reply == NULL, cleanup);

    val = json_object_get(reply, "username");
    goto_if_true(val == NULL || !json_is_string(val), cleanup);
    dc_account_set_username(user, json_string_value(val));

    val = json_object_get(reply, "discriminator");
    goto_if_true(val == NULL || !json_is_string(val), cleanup);
    dc_account_set_discriminator(user, json_string_value(val));

    val = json_object_get(reply, "id");
    goto_if_true(val == NULL || !json_is_string(val), cleanup);
    dc_account_set_id(user, json_string_value(val));

    ret = true;

cleanup:

    free(url);
    json_decref(reply);

    return ret;
}

bool dc_api_get_userguilds(dc_api_t api, dc_account_t login, GPtrArray **out)
{
    char const *url = "users/@me/guilds";
    json_t *reply = NULL, *c = NULL, *val = NULL;
    size_t i = 0;
    bool ret = false;
    GPtrArray *guilds = g_ptr_array_new_with_free_func(
        (GDestroyNotify)dc_unref
        );

    return_if_true(api == NULL, false);
    return_if_true(login == NULL, false);

    reply = dc_api_call_sync(api, "GET", dc_account_token(login), url, NULL);
    goto_if_true(reply == NULL, cleanup);

    goto_if_true(!json_is_array(reply), cleanup);

    json_array_foreach(reply, i, c) {
        dc_guild_t g = dc_guild_new();

        val = json_object_get(c, "id");
        goto_if_true(val == NULL || !json_is_string(val), cleanup);
        dc_guild_set_id(g, json_string_value(val));

        val = json_object_get(c, "name");
        goto_if_true(val == NULL || !json_is_string(val), cleanup);
        dc_guild_set_name(g, json_string_value(val));

        g_ptr_array_add(guilds, g);
    }

    *out = guilds;
    guilds = NULL;

    ret = true;

cleanup:

    json_decref(reply);

    if (guilds) {
        g_ptr_array_unref(guilds);
        guilds = NULL;
    }

    return ret;
}
