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

#include <ncdc/cmds.h>

ncdc_commands_t cmds[] = {
    { L"/ack",      ncdc_cmd_ack },
    { L"/close",    ncdc_cmd_close },
    { L"/connect",  ncdc_cmd_login },
    { L"/friend",   ncdc_cmd_friends },
    { L"/friends",  ncdc_cmd_friends },
    { L"/join",     ncdc_cmd_join },
    { L"/login",    ncdc_cmd_login },
    { L"/logout",   ncdc_cmd_logout },
    { L"/markread", ncdc_cmd_ack },
    { L"/msg",      ncdc_cmd_msg },
    { L"/post",     ncdc_cmd_post },
    { L"/quit",     ncdc_cmd_quit },
    { L"/wc",       ncdc_cmd_close },
    { NULL, NULL }
};

static GQueue *queue = NULL;
static pthread_t thr;
static pthread_mutex_t mtx;
static pthread_cond_t cnd;

typedef struct {
    ncdc_commands_t *cmd;
    wchar_t *f;
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
                item->cmd->handler(item->mainwindow, item->ac,
                                   item->av, item->f
                    );

                w_strfreev(item->av);
                free(item->f);
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

ncdc_commands_t *ncdc_find_cmd(ncdc_commands_t *cmds, wchar_t const *name)
{
    ncdc_commands_t *it = NULL;

    for (it = cmds; it->name != NULL; it++) {
        if (wcscmp(it->name, name) == 0) {
            return it;
        }
    }

    return NULL;
}

bool ncdc_dispatch(ncdc_mainwindow_t n, wchar_t const *s)
{
    wchar_t **tokens = NULL;
    ncdc_commands_t *it = NULL;
    queue_item *item = NULL;
    wchar_t *f = NULL;
    size_t len = 0, cmdlen = 0;

    tokens = w_tokenise(s);
    return_if_true(tokens == NULL, false);

    if ((it = ncdc_find_cmd(cmds, tokens[0])) == NULL) {
        /* no such command
         */
        LOG(n, L"error: no such command \"%ls\"", tokens[0]);
        w_strfreev(tokens);
        return false;
    }

    /* make a complete string without the /command part
     */
    len = wcslen(s);
    cmdlen = wcslen(it->name);

    f = wcsdup(s);
    return_if_true(f == NULL, false);
    memmove(f, f+cmdlen, (len-cmdlen) * sizeof(wchar_t));
    f[len-cmdlen] = '\0';

    item = calloc(1, sizeof(queue_item));

    item->ac = w_strlenv(tokens);
    item->av = tokens;
    item->cmd = it;
    item->mainwindow = n;
    item->f = f;

    pthread_mutex_lock(&mtx);
    g_queue_push_tail(queue, item);
    pthread_cond_broadcast(&cnd);
    pthread_mutex_unlock(&mtx);

    return true;
}

bool ncdc_cmd_quit(ncdc_mainwindow_t n, size_t ac,
                   wchar_t **av, wchar_t const *f)
{
    exit_main();
    return true;
}
