#include <dc/api.h>
#include <dc/refable.h>
#include "internal.h"

#define DISCORD_URL "https://discordapp.com/api/v6"

#define DISCORD_USERAGENT "Mozilla/5.0 (X11; Linux x86_64; rv:67.0) Gecko/20100101 Firefox/67.0"
#define DISCORD_API_AUTH "auth/login"

struct dc_api_
{
    dc_refable_t ref;

    struct event_base *base;
    CURLM *curl;

    GHashTable *syncs;

    char *cookie;
};

static void dc_api_free(dc_api_t ptr)
{
    return_if_true(ptr == NULL,);

    if (ptr->syncs != NULL) {
        g_hash_table_unref(ptr->syncs);
        ptr->syncs = NULL;
    }

    free(ptr);
}

dc_api_t dc_api_new(void)
{
    dc_api_t ptr = calloc(1, sizeof(struct dc_api_));
    return_if_true(ptr == NULL, NULL);

    ptr->ref.cleanup = (dc_cleanup_t)dc_api_free;

    ptr->syncs = g_hash_table_new_full(g_direct_hash, g_direct_equal,
                                       NULL, dc_unref
        );
    if (ptr->syncs == NULL) {
        free(ptr);
        return NULL;
    }

    return dc_ref(ptr);
}

void dc_api_set_curl_multi(dc_api_t api, CURLM *curl)
{
    return_if_true(api == NULL,);
    return_if_true(curl == NULL,);

    api->curl = curl;
}

void dc_api_set_event_base(dc_api_t api, struct event_base *base)
{
    return_if_true(api == NULL,);
    return_if_true(base == NULL,);

    api->base = base;
}

void dc_api_signal(dc_api_t api, CURL *easy, int code)
{
    dc_api_sync_t sync = NULL;

    return_if_true(api == NULL,);
    return_if_true(easy == NULL,);

    sync = g_hash_table_lookup(api->syncs, easy);
    if (sync != NULL) {
        dc_api_sync_finish(sync, code);
        g_hash_table_remove(api->syncs, easy);
    }
}

#ifdef DEBUG
static int debug_callback(CURL *handle, curl_infotype type,
                          char *data, size_t size,
                          void *userptr
    )
{
    switch (type) {
    case CURLINFO_TEXT: printf("+T: %s", data); break;
    case CURLINFO_HEADER_IN: printf(">H: %s", data); break;
    case CURLINFO_HEADER_OUT: printf("<H: %s", data); break;
    case CURLINFO_DATA_IN: printf(">D: %s\n", data); break;
    case CURLINFO_DATA_OUT: printf("<D: %s\n", data); break;
    case CURLINFO_SSL_DATA_IN:
    case CURLINFO_SSL_DATA_OUT:
    default: break;
    }

    return 0;
}
#endif

static dc_api_sync_t
dc_api_do(dc_api_t api, char const *verb,
          char const *url, char const *token,
          char const *data, int64_t len)
{
    return_if_true(api == NULL, NULL);
    return_if_true(api->curl == NULL, NULL);
    return_if_true(url == NULL, NULL);
    return_if_true(verb == NULL, NULL);

    CURL *c = NULL;
    bool ret = false;
    dc_api_sync_t sync = NULL;
    struct curl_slist *l = NULL;
    char *tmp = NULL;
    int ptr = 0;

    c = curl_easy_init();
    goto_if_true(c == NULL, cleanup);

    sync = dc_api_sync_new(api->curl, c);
    goto_if_true(c == NULL, cleanup);

    curl_easy_setopt(c, CURLOPT_URL, url);
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, fwrite);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, dc_api_sync_stream(sync));

    if (api->cookie != NULL) {
        curl_easy_setopt(c, CURLOPT_COOKIE, api->cookie);
    }

    l = dc_api_sync_list(sync);
    if (data != NULL) {
        curl_slist_append(l, "Content-Type: application/json");
    }
    curl_slist_append(l, "Accept: application/json");
    curl_slist_append(l, "User-Agent: " DISCORD_USERAGENT);
    curl_slist_append(l, "Pragma: no-cache");
    curl_slist_append(l, "Cache-Control: no-cache");

    if (token != NULL) {
        asprintf(&tmp, "Authorization: %s", token);
        curl_slist_append(l, tmp);
        free(tmp);
        tmp = NULL;
    }

    curl_easy_setopt(c, CURLOPT_HTTPHEADER, l);
    curl_easy_setopt(c, CURLOPT_FORBID_REUSE, 1L);
    //curl_easy_setopt(c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);

#ifdef DEBUG
    curl_easy_setopt(c, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(c, CURLOPT_DEBUGFUNCTION, debug_callback);
#endif

    if (strcmp(verb, "POST") == 0) {
        curl_easy_setopt(c, CURLOPT_POST, 1UL);
        curl_easy_setopt(c, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);
    }

    if (data != NULL) {
        curl_easy_setopt(c, CURLOPT_COPYPOSTFIELDS, data);
        if (len >= 0) {
            curl_easy_setopt(c, CURLOPT_POSTFIELDSIZE_LARGE, len);
        }
    }

    if (strcmp(verb, "PUT") == 0 ||
        strcmp(verb, "DELETE") == 0) {
        curl_easy_setopt(c, CURLOPT_CUSTOMREQUEST, verb);
    }

    if (curl_multi_add_handle(api->curl, c) != CURLM_OK) {
        goto cleanup;
    }

    g_hash_table_insert(api->syncs, c, dc_ref(sync));
    curl_multi_socket_action(api->curl, CURL_SOCKET_TIMEOUT, 0, &ptr);

    ret = true;

cleanup:

    if (!ret) {
        dc_unref(sync);
        sync = NULL;
    }

    return sync;
}

dc_api_sync_t dc_api_call(dc_api_t api, char const *token,
                          char const *verb, char const *method,
                          json_t *j)
{
    char *data = NULL;
    char *url = NULL;
    dc_api_sync_t s = NULL;

    asprintf(&url, "%s/%s", DISCORD_URL, method);
    goto_if_true(url == NULL, cleanup);

    if (j != NULL) {
        data = json_dumps(j, JSON_COMPACT);
        goto_if_true(data == NULL, cleanup);
    }

    s = dc_api_do(api, verb, url, token, data, -1);
    goto_if_true(s == NULL, cleanup);

cleanup:

    free(data);
    data = NULL;

    free(url);
    url = NULL;

    return s;
}

json_t *dc_api_call_sync(dc_api_t api, char const *token,
                         char const *verb, char const *method,
                         json_t *j)
{
    dc_api_sync_t s = NULL;
    json_t *reply = NULL;

    s = dc_api_call(api, verb, token, method, j);
    goto_if_true(s == NULL, cleanup);

    if (!dc_api_sync_wait(s)) {
        goto cleanup;
    }

#ifdef DEBUG
    printf("api_call_sync: %d\n", dc_api_sync_code(s));
#endif

    reply = json_loadb(dc_api_sync_data(s),
                       dc_api_sync_datalen(s),
                       0, NULL
        );

cleanup:

    dc_unref(s);
    s = NULL;

    return reply;
}

static bool dc_api_error(json_t *j, int *code, char const **message)
{
    return_if_true(j == NULL, false);

    bool error = false;
    json_t *c = NULL, *m = NULL;

    c = json_object_get(j, "code");
    if (c != NULL) {
        error = true;
        if (code != NULL) {
            *code = json_integer_value(c);
        }
    }

    m = json_object_get(j, "message");
    if (m != NULL) {
        error = true;
        if (message != NULL) {
            *message = json_string_value(m);
        }
    }

    return error;
}

bool dc_api_authenticate(dc_api_t api, dc_account_t account)
{
    json_t *j = json_object(), *reply = NULL, *token = NULL;
    bool ret = false;

    json_object_set_new(j, "email",
                        json_string(dc_account_email(account))
        );
    json_object_set_new(j, "password",
                        json_string(dc_account_password(account))
        );

    reply = dc_api_call_sync(api, "POST", NULL, DISCORD_API_AUTH, j);
    goto_if_true(reply == NULL, cleanup);

    if (dc_api_error(j, NULL, NULL)) {
        return false;
    }

    token = json_object_get(reply, "token");
    if (token == NULL || !json_is_string(token)) {
        goto cleanup;
    }

    dc_account_set_token(account, json_string_value(token));
    ret = true;

cleanup:

    if (j != NULL) {
        json_decref(j);
        j = NULL;
    }

    if (reply != NULL) {
        json_decref(reply);
        reply = NULL;
    }

    return ret;
}

static dc_account_t dc_api_account_from_json(json_t *j)
{
    dc_account_t user = dc_account_new();
    json_t *val = NULL;

    goto_if_true(!json_is_object(j), error);

    val = json_object_get(j, "username");
    goto_if_true(val == NULL || !json_is_string(val), error);
    dc_account_set_username(user, json_string_value(val));

    val = json_object_get(j, "discriminator");
    goto_if_true(val == NULL || !json_is_string(val), error);
    dc_account_set_discriminator(user, json_string_value(val));

    val = json_object_get(j, "id");
    goto_if_true(val == NULL || !json_is_string(val), error);
    dc_account_set_id(user, json_string_value(val));

    return user;

error:

    dc_unref(user);
    return NULL;
}

static json_t *dc_api_user_to_json(dc_account_t a)
{
    json_t *j = NULL;

    return_if_true(a == NULL, NULL);
    return_if_true(dc_account_username(a) == NULL ||
                   dc_account_discriminator(a) == NULL,
                   NULL
        );

    j = json_object();
    return_if_true(j == NULL, NULL);

    json_object_set_new(j, "username",
                        json_string(dc_account_username(a))
        );
    json_object_set_new(j, "discriminator",
                        json_string(dc_account_discriminator(a))
        );

    return j;
}

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

/**
 * Add a given account as a friend to the friends list
 */
bool dc_api_add_friend(dc_api_t api, dc_account_t login, dc_account_t friend)
{
    char const *url = "users/@me/relationships";
    json_t *reply = NULL, *post = NULL;
    bool ret = false;

    return_if_true(api == NULL, false);
    return_if_true(login == NULL, false);

    post = dc_api_user_to_json(friend);
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
