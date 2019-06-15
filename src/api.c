#include <ncdc/api.h>
#include <ncdc/refable.h>

#define DISCORD_URL "https://discordapp.com/api/v6"

#define DISCORD_USERAGENT "Mozilla/5.0 (X11; Linux x86_64; rv:67.0) Gecko/20100101 Firefox/67.0"
#define DISCORD_API_AUTH "auth/login"

struct ncdc_api_
{
    ncdc_refable_t ref;

    struct event_base *base;
    CURLM *curl;

    GHashTable *syncs;

    char *cookie;
};

static void ncdc_api_free(ncdc_api_t ptr)
{
    return_if_true(ptr == NULL,);

    if (ptr->syncs != NULL) {
        g_hash_table_unref(ptr->syncs);
        ptr->syncs = NULL;
    }

    free(ptr);
}

ncdc_api_t ncdc_api_new(void)
{
    ncdc_api_t ptr = calloc(1, sizeof(struct ncdc_api_));
    return_if_true(ptr == NULL, NULL);

    ptr->ref.cleanup = (cleanup_t)ncdc_api_free;

    ptr->syncs = g_hash_table_new_full(g_direct_hash, g_direct_equal,
                                       NULL, ncdc_unref
        );
    if (ptr->syncs == NULL) {
        free(ptr);
        return NULL;
    }

    return ncdc_ref(ptr);
}

void ncdc_api_set_curl_multi(ncdc_api_t api, CURLM *curl)
{
    return_if_true(api == NULL,);
    return_if_true(curl == NULL,);

    api->curl = curl;
}

void ncdc_api_set_event_base(ncdc_api_t api, struct event_base *base)
{
    return_if_true(api == NULL,);
    return_if_true(base == NULL,);

    api->base = base;
}

void ncdc_api_signal(ncdc_api_t api, CURL *easy, int code)
{
    ncdc_api_sync_t sync = NULL;

    return_if_true(api == NULL,);
    return_if_true(easy == NULL,);

    sync = g_hash_table_lookup(api->syncs, easy);
    if (sync != NULL) {
        ncdc_api_sync_finish(sync, code);
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

static int header_callback(char *data, size_t sz, size_t num, ncdc_api_t api)
{
    char *ptr =  NULL;

    if ((ptr = strstr(data, "set-cookie")) != NULL) {
        free(api->cookie);
        api->cookie = NULL;

        if ((ptr = strstr(data, ":")) != NULL) {
            api->cookie = strdup(ptr+1);
            if ((ptr = strstr(api->cookie, ";")) != NULL) {
                *ptr = '\0';
            }
        }
    }

    return sz * num;
}

static ncdc_api_sync_t ncdc_api_post(ncdc_api_t api,
                                     char const *url,
                                     char const *token,
                                     char const *data, int64_t len)
{
    return_if_true(api == NULL, NULL);
    return_if_true(api->curl == NULL, NULL);
    return_if_true(url == NULL, NULL);

    CURL *c = NULL;
    bool ret = false;
    ncdc_api_sync_t sync = NULL;
    struct curl_slist *l = NULL;
    char *tmp = NULL;
    int ptr = 0;

    c = curl_easy_init();
    goto_if_true(c == NULL, cleanup);

    sync = ncdc_api_sync_new(api->curl, c);
    goto_if_true(c == NULL, cleanup);

    curl_easy_setopt(c, CURLOPT_URL, url);
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, fwrite);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, ncdc_api_sync_stream(sync));
    curl_easy_setopt(c, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(c, CURLOPT_HEADERDATA, api);

    if (api->cookie != NULL) {
        curl_easy_setopt(c, CURLOPT_COOKIE, api->cookie);
    }

    l = ncdc_api_sync_list(sync);
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

    if (data != NULL) {
        curl_easy_setopt(c, CURLOPT_POST, 1UL);
        curl_easy_setopt(c, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);
        curl_easy_setopt(c, CURLOPT_COPYPOSTFIELDS, data);
        if (len >= 0) {
            curl_easy_setopt(c, CURLOPT_POSTFIELDSIZE_LARGE, len);
        }
    }

    if (curl_multi_add_handle(api->curl, c) != CURLM_OK) {
        goto cleanup;
    }

    g_hash_table_insert(api->syncs, c, ncdc_ref(sync));
    curl_multi_socket_action(api->curl, CURL_SOCKET_TIMEOUT, 0, &ptr);

    ret = true;

cleanup:

    if (!ret) {
        ncdc_unref(sync);
        sync = NULL;
    }

    return sync;
}

ncdc_api_sync_t ncdc_api_call(ncdc_api_t api, char const *token,
                              char const *method, json_t *j)
{
    char *data = NULL;
    char *url = NULL;
    ncdc_api_sync_t s = NULL;

    asprintf(&url, "%s/%s", DISCORD_URL, method);
    goto_if_true(url == NULL, cleanup);

    if (j != NULL) {
        data = json_dumps(j, JSON_COMPACT);
        goto_if_true(data == NULL, cleanup);
    }

    s = ncdc_api_post(api, url, token, data, -1);
    goto_if_true(s == NULL, cleanup);

cleanup:

    free(data);
    data = NULL;

    free(url);
    url = NULL;

    return s;
}

json_t *ncdc_api_call_sync(ncdc_api_t api, char const *token,
                           char const *method, json_t *j)
{
    ncdc_api_sync_t s = NULL;
    json_t *reply = NULL;

    s = ncdc_api_call(api, token, method, j);
    goto_if_true(s == NULL, cleanup);

    if (!ncdc_api_sync_wait(s)) {
        goto cleanup;
    }

#ifdef DEBUG
    printf("api_call_sync: %d\n", ncdc_api_sync_code(s));
#endif

    reply = json_loadb(ncdc_api_sync_data(s),
                       ncdc_api_sync_datalen(s),
                       0, NULL
        );

cleanup:

    ncdc_unref(s);
    s = NULL;

    return reply;
}

static bool ncdc_api_error(json_t *j, int *code, char const **message)
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

bool ncdc_api_authenticate(ncdc_api_t api, ncdc_account_t account)
{
    json_t *j = json_object(), *reply = NULL, *token = NULL;
    bool ret = false;

    json_object_set_new(j, "email",
                        json_string(ncdc_account_email(account))
        );
    json_object_set_new(j, "password",
                        json_string(ncdc_account_password(account))
        );

    reply = ncdc_api_call_sync(api, NULL, DISCORD_API_AUTH, j);
    goto_if_true(reply == NULL, cleanup);

    if (ncdc_api_error(j, NULL, NULL)) {
        return false;
    }

    token = json_object_get(reply, "token");
    if (token == NULL || !json_is_string(token)) {
        goto cleanup;
    }

    ncdc_account_set_token(account, json_string_value(token));
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

bool ncdc_api_userinfo(ncdc_api_t api, ncdc_account_t login,
                       ncdc_account_t user)
{
    char *url = NULL;
    json_t *reply = NULL;
    bool ret = false;

    return_if_true(api == NULL, false);
    return_if_true(login == NULL, false);
    return_if_true(user == NULL, false);

    asprintf(&url, "users/%s", ncdc_account_id(user));

    reply = ncdc_api_call_sync(api, ncdc_account_token(login), url, NULL);
    goto_if_true(reply == NULL, cleanup);

    /* TODO: parse json and store info in user
     */

    ret = true;

cleanup:

    if (reply != NULL) {
        json_decref(reply);
        reply = NULL;
    }

    return ret;
}
