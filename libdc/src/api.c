#include <dc/api.h>
#include <dc/refable.h>
#include "internal.h"

#define DISCORD_URL     "https://discordapp.com/api/v6"
#define DISCORD_GATEWAY "https://gateway.discord.gg/?encoding=json&v=6"

#define DISCORD_USERAGENT "Mozilla/5.0 (X11; Linux x86_64; rv:67.0) Gecko/20100101 Firefox/67.0"

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

int debug_callback(CURL *handle, curl_infotype type,
                   char *data, size_t size,
                   void *userptr)
{
    FILE *f = fopen("debug.log", "a+");

    switch (type) {
    case CURLINFO_TEXT: fprintf(f, "+T: %s", data); break;
    case CURLINFO_HEADER_IN: fprintf(f, ">H: %s", data); break;
    case CURLINFO_HEADER_OUT: fprintf(f, "<H: %s", data); break;
    case CURLINFO_DATA_IN: fprintf(f, ">D: %s\n", data); break;
    case CURLINFO_DATA_OUT: fprintf(f, "<D: %s\n", data); break;
    case CURLINFO_SSL_DATA_IN: fprintf(f, "<S: SSL_DATA_IN\n"); break;
    case CURLINFO_SSL_DATA_OUT: fprintf(f, "<S: SSL_DATA_OUT\n"); break;
    default: break;
    }

    fclose(f);

    return 0;
}

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
        if (len >= 0) {
            curl_easy_setopt(c, CURLOPT_POSTFIELDSIZE_LARGE, len);
        }
        curl_easy_setopt(c, CURLOPT_COPYPOSTFIELDS, data);
    }

    if (strcmp(verb, "PUT") == 0 ||
        strcmp(verb, "DELETE") == 0) {
        curl_easy_setopt(c, CURLOPT_CUSTOMREQUEST, verb);
    }

    if (curl_multi_add_handle(api->curl, c) != CURLM_OK) {
        goto cleanup;
    }

    g_hash_table_insert(api->syncs, c, dc_ref(sync));
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

bool dc_api_error(json_t *j, int *code, char const **message)
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

static size_t stall_connection(char *buffer, size_t size, size_t nitems,
                               void *userdata)
{
    CURL *easy = (CURL *)userdata;

    if (strncmp(buffer, "\r\n", size) == 0) {
        curl_easy_setopt(easy, CURLOPT_CONNECT_ONLY, 1);
        //curl_easy_pause(easy, CURLPAUSE_ALL);
        curl_easy_setopt(easy, CURLOPT_FORBID_REUSE, 1);
    }

    return size * nitems;
}

dc_gateway_t dc_api_establish_gateway(dc_api_t api, dc_account_t login)
{
    return_if_true(api == NULL, NULL);
    return_if_true(api->curl == NULL, NULL);
    return_if_true(login == NULL || !dc_account_has_token(login), NULL);

    CURL *c = NULL;
    struct curl_slist *list = NULL;
    /* BE THE BROKEN OR THE BREAKER
     */
    dc_gateway_t gw = NULL;
    dc_gateway_t ret = NULL;

    c = curl_easy_init();
    goto_if_true(c == NULL, cleanup);

    gw = dc_gateway_new();
    goto_if_true(gw == NULL, cleanup);

    curl_easy_setopt(c, CURLOPT_URL, DISCORD_GATEWAY);

    list = dc_gateway_slist(gw);
    curl_slist_append(list, "Content-Type: application/json");
    curl_slist_append(list, "Accept: application/json");
    curl_slist_append(list, "User-Agent: " DISCORD_USERAGENT);
    curl_slist_append(list, "Pragma: no-cache");
    curl_slist_append(list, "Cache-Control: no-cache");
    curl_slist_append(list, "Sec-WebSocket-Key: cbYK1Jm6cpk3Rua");
    curl_slist_append(list, "Sec-WebSocket-Version: 13");
    curl_slist_append(list, "Upgrade: websocket");

    curl_easy_setopt(c, CURLOPT_HEADERFUNCTION, stall_connection);
    curl_easy_setopt(c, CURLOPT_HEADERDATA, c);

    curl_easy_setopt(c, CURLOPT_HTTPHEADER, list);

    curl_easy_setopt(c, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(c, CURLOPT_TCP_KEEPIDLE, 120L);
    curl_easy_setopt(c, CURLOPT_TCP_KEEPINTVL, 60L);

    curl_easy_setopt(c, CURLOPT_FORBID_REUSE, 1L);
    curl_easy_setopt(c, CURLOPT_FRESH_CONNECT, 1L);

    curl_easy_setopt(c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, dc_gateway_writefunc);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, gw);

    dc_gateway_set_login(gw, login);
    dc_gateway_set_curl(gw, api->curl, c);

    if (curl_multi_add_handle(api->curl, c) != CURLM_OK) {
        goto cleanup;
    }

    c = NULL;

    ret = gw;
    gw = NULL;

cleanup:

    if (c != NULL) {
        curl_easy_cleanup(c);
    }

    dc_unref(gw);

    return ret;
}
