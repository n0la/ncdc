#include <ncdc/cmds.h>

ncdc_commands_t cmds[] = {
    { L"/friend",  ncdc_cmd_friends },
    { L"/friends", ncdc_cmd_friends },
    { L"/login",   ncdc_cmd_login },
    { L"/quit",    ncdc_cmd_quit },
    { NULL, NULL }
};

static GQueue *queue = NULL;
static pthread_t thr;
static pthread_mutex_t mtx;
static pthread_cond_t cnd;

typedef struct {
    ncdc_commands_t *cmd;
    size_t ac;
    wchar_t **av;
    ncdc_mainwindow_t mainwindow;
} queue_item;

static void *async_dispatcher(void *arg)
{
    queue_item *item = NULL;

    while (true) {
        pthread_mutex_lock(&mtx);
        pthread_cond_wait(&cnd, &mtx);

        /* of course it is "get_length", and not "size" or
         * "length", or something else. srsly.
         */
        while (g_queue_get_length(queue) > 0) {
            item = g_queue_pop_head(queue);
            if (item == NULL) {
                /* end of working orders
                 */
                pthread_mutex_unlock(&mtx);
                return NULL;
            } else {
                /* call the handler
                 */
                item->cmd->handler(item->mainwindow, item->ac, item->av);

                w_strfreev(item->av);
                free(item);
            }
        }

        pthread_mutex_unlock(&mtx);
    }

    return NULL;
}

bool ncdc_dispatch_init(void)
{
    return_if_true(queue != NULL, true);

    queue = g_queue_new();
    return_if_true(queue == NULL, false);

    pthread_mutex_init(&mtx, NULL);
    pthread_cond_init(&cnd, NULL);

    pthread_create(&thr, NULL, async_dispatcher, NULL);

    return true;
}

bool ncdc_dispatch_deinit(void)
{
    return_if_true(queue == NULL, true);

    pthread_mutex_lock(&mtx);
    g_queue_push_tail(queue, NULL);
    pthread_cond_signal(&cnd);
    pthread_mutex_unlock(&mtx);

    pthread_join(thr, NULL);

    pthread_mutex_destroy(&mtx);
    pthread_cond_destroy(&cnd);

    g_queue_free(queue);
    queue = NULL;

    return true;
}

bool ncdc_dispatch(ncdc_mainwindow_t n, wchar_t const *s)
{
    wchar_t **tokens = NULL;
    size_t i = 0, tokenlen = 0;
    ncdc_commands_t *it = NULL;
    queue_item *item = NULL;

    tokens = w_tokenise(s);
    return_if_true(tokens == NULL, false);

    tokenlen = wcslen(tokens[0]);

    for (i = 0; cmds[i].name != NULL; i++) {
        if (wcsncmp(cmds[i].name, tokens[0], tokenlen) == 0) {
            it = cmds+i;
            break;
        }
    }

    if (it == NULL) {
        /* no such command
         */
        LOG(n, L"error: no such command \"%ls\"", tokens[0]);
        w_strfreev(tokens);
        return false;
    }

    item = calloc(1, sizeof(queue_item));

    item->ac = w_strlenv(tokens);
    item->av = tokens;
    item->cmd = it;
    item->mainwindow = n;

    pthread_mutex_lock(&mtx);
    g_queue_push_tail(queue, item);
    pthread_cond_broadcast(&cnd);
    pthread_mutex_unlock(&mtx);

    return true;
}

bool ncdc_cmd_quit(ncdc_mainwindow_t n, size_t ac, wchar_t **av)
{
    exit_main();
    return true;
}
