#include <dc/api.h>

#include "internal.h"

bool dc_api_get_friends(dc_api_t api, dc_account_t login)
{
    char const *url = "users/@me/relationships";
    json_t *reply = NULL, *c = NULL, *val = NULL;
    bool ret = false;
    size_t i = 0;
    GPtrArray *f = g_ptr_array_new_with_free_func((GDestroyNotify)dc_unref);

    return_if_true(api == NULL, false);
    return_if_true(login == NULL, false);

    reply = dc_api_call_sync(api, "GET", dc_account_token(login), url, NULL);
    goto_if_true(reply == NULL, cleanup);

    goto_if_true(!json_is_array(reply), cleanup);

    json_array_foreach(reply, i, c) {
        /* the return is an array of objects, with a "user" member
         * type 1 is probably a friend
         */
        val = json_object_get(c, "user");
        if (val == NULL) {
            continue;
        }

        dc_account_t a = dc_api_account_from_json(val);
        if (a == NULL) {
            continue;
        }

        /* read the type also known as "typ"
         */
        val = json_object_get(c, "type");
        if (val != NULL && json_is_integer(val)) {
            int state = json_integer_value(val);
            dc_account_set_friend_state(a, state);
        }

        g_ptr_array_add(f, a);
    }

    if (f->len == 0) {
        /* me_irl :-(
         */
    }

    dc_account_set_friends(login, (dc_account_t*)f->pdata, f->len);
    ret = true;

cleanup:

    g_ptr_array_free(f, FALSE);
    json_decref(reply);
    reply = NULL;

    return ret;
}

bool dc_api_remove_friend(dc_api_t api, dc_account_t login, dc_account_t friend)
{
    char *url = NULL;
    json_t *reply = NULL, *post = NULL;
    bool ret = false;

    return_if_true(api == NULL, false);
    return_if_true(login == NULL || friend == NULL, false);
    return_if_true(dc_account_id(friend) == NULL, false);

    asprintf(&url, "users/@me/relationships/%s", dc_account_id(friend));

    post = dc_api_account_to_json(friend);
    return_if_true(post == NULL, false);

    reply = dc_api_call_sync(api, "DELETE", dc_account_token(login), url, post);
    /* if no data comes back, then the whole thing was a success
     */
    goto_if_true(reply != NULL, cleanup);

    ret = true;

cleanup:

    free(url);
    json_decref(post);
    json_decref(reply);

    return ret;
}

bool dc_api_accept_friend(dc_api_t api, dc_account_t login, dc_account_t friend)
{
    char *url = NULL;
    json_t *reply = NULL, *post = NULL;
    bool ret = false;

    return_if_true(api == NULL, false);
    return_if_true(login == NULL || friend == NULL, false);
    return_if_true(dc_account_id(friend) == NULL, false);

    asprintf(&url, "users/@me/relationships/%s", dc_account_id(friend));

    post = dc_api_account_to_json(friend);
    return_if_true(post == NULL, false);

    reply = dc_api_call_sync(api, "PUT", dc_account_token(login), url, post);
    /* no data = successful
     */
    goto_if_true(reply != NULL, cleanup);

    ret = true;

cleanup:

    free(url);
    json_decref(post);
    json_decref(reply);

    return ret;
}

bool dc_api_add_friend(dc_api_t api, dc_account_t login, dc_account_t friend)
{
    char const *url = "users/@me/relationships";
    json_t *reply = NULL, *post = NULL;
    bool ret = false;

    return_if_true(api == NULL, false);
    return_if_true(login == NULL, false);

    post = dc_api_account_to_json(friend);
    return_if_true(post == NULL, false);

    reply = dc_api_call_sync(api, "POST", dc_account_token(login), url, post);
    /* apparently if no data comes back, then the whole thing was a success
     */
    goto_if_true(reply != NULL, cleanup);

    ret = true;

cleanup:

    json_decref(post);
    json_decref(reply);

    return ret;
}
