#include <ncdc/mainwindow.h>
#include <ncdc/input.h>
#include <ncdc/textview.h>
#include <ncdc/treeview.h>
#include <ncdc/cmds.h>
#include <ncdc/ncdc.h>

typedef enum {
    FOCUS_GUILDS = 0,
    FOCUS_CHAT,
    FOCUS_INPUT,
} focus_t;

struct ncdc_mainwindow_
{
    dc_refable_t ref;

    WINDOW *guilds;
    int guilds_w;
    int guilds_h;
    int guilds_y;
    int guilds_x;

    WINDOW *chat;
    int chat_h;
    int chat_w;
    int chat_y;
    int chat_x;

    WINDOW *input;
    int input_w;
    int input_h;
    int input_y;
    int input_x;
    int input_curs_x;

    WINDOW *sep1;
    WINDOW *sep2;

    ncdc_input_t in;
    ncdc_treeview_t guildview;
    ncdc_treeitem_t root;

    GPtrArray *views;
    int curview;
    ncdc_textview_t log;

    int focus;
};

static void ncdc_mainwindow_resize(ncdc_mainwindow_t n);
static void ncdc_mainwindow_update_focus(ncdc_mainwindow_t n);
static bool ncdc_mainwindow_callback(ncdc_input_t i, wchar_t const *s,
                                     size_t len, void *arg);

static void ncdc_mainwindow_free(ncdc_mainwindow_t n)
{
    return_if_true(n == NULL,);

    delwin(n->guilds);
    delwin(n->chat);
    delwin(n->input);

    delwin(n->sep1);
    delwin(n->sep2);

    dc_unref(n->in);
    dc_unref(n->guildview);

    if (n->views != NULL) {
        g_ptr_array_unref(n->views);
        n->views = NULL;
    }

    free(n);
}

ncdc_mainwindow_t ncdc_mainwindow_new(void)
{
    ncdc_mainwindow_t ptr = calloc(1, sizeof(struct ncdc_mainwindow_));
    return_if_true(ptr == NULL, NULL);

    ptr->ref.cleanup = (dc_cleanup_t)ncdc_mainwindow_free;

    ptr->in = ncdc_input_new();
    ncdc_input_set_callback(ptr->in, ncdc_mainwindow_callback, ptr);

    ptr->guildview = ncdc_treeview_new();
    ptr->root = ncdc_treeview_root(ptr->guildview);

    ptr->views = g_ptr_array_new_with_free_func(
        (GDestroyNotify)dc_unref
        );
    ptr->log = ncdc_textview_new();
    ncdc_textview_set_title(ptr->log, L"status");
    g_ptr_array_add(ptr->views, ptr->log);

    ptr->guilds = newwin(5, 5, 1, 1);
    ptr->chat = newwin(5, 5, 4, 4);

    ptr->input = newwin(5, 5, 8, 8);
    keypad(ptr->input, TRUE);

    ptr->sep1 = newwin(5, 5, 10, 10);
    ptr->sep2 = newwin(5, 5, 12, 12);

    ncdc_mainwindow_resize(ptr);

    ptr->focus = FOCUS_INPUT;
    ncdc_mainwindow_update_focus(ptr);

    return dc_ref(ptr);
}

static bool
ncdc_mainwindow_callback(ncdc_input_t i, wchar_t const *s,
                         size_t len, void *arg)
{
    ncdc_mainwindow_t mainwin = (ncdc_mainwindow_t)arg;
    bool ret = false;

    if (s == NULL || s[0] == '\0') {
        return false;
    }

    if (s[0] == '/') {
        ret = ncdc_dispatch(mainwin, s);
    } else {
        wchar_t *post = NULL;

        aswprintf(&post, L"/post %ls", s);
        ret = ncdc_dispatch(mainwin, post);
        free(post);
    }

    return ret;
}

static void ncdc_mainwindow_resize(ncdc_mainwindow_t n)
{
    n->guilds_h = LINES - 2;
    n->guilds_w = (COLS / 4);
    n->guilds_y = 0;
    n->guilds_x = 0;

    wresize(n->guilds, n->guilds_h, n->guilds_w);
    mvwin(n->guilds, n->guilds_y, n->guilds_x);
    wnoutrefresh(n->guilds);

    n->input_h = 1;
    n->input_w = COLS;
    n->input_y = LINES - n->input_h;
    n->input_x = 0;

    wresize(n->input, n->input_h, n->input_w);
    mvwin(n->input, n->input_y, n->input_x);
    wnoutrefresh(n->input);

    wresize(n->sep1, 1, COLS);
    mvwin(n->sep1, LINES - 2, 0);
    wbkgd(n->sep1, COLOR_PAIR(1));
    wnoutrefresh(n->sep1);

    n->chat_h = LINES - n->input_h - 1;
    n->chat_w = COLS - n->guilds_w - 2;
    n->chat_y = 0;
    n->chat_x = n->guilds_w + 2;

    wresize(n->chat, n->chat_h, n->chat_w);
    mvwin(n->chat, n->chat_y, n->chat_x);
    wnoutrefresh(n->chat);

    wresize(n->sep2, LINES - 2, 1);
    mvwin(n->sep2, 0, n->guilds_w + 1);
    wnoutrefresh(n->sep2);

    ncdc_mainwindow_update_focus(n);
}

static void ncdc_mainwindow_update_focus(ncdc_mainwindow_t n)
{
    switch (n->focus) {
    case FOCUS_GUILDS:
    {
        curs_set(0);
    } break;

    case FOCUS_CHAT:
    {
        curs_set(0);
    } break;

    case FOCUS_INPUT:
    {
        curs_set(1);
        wmove(n->input, 0, ncdc_input_cursor(n->in));
        wrefresh(n->input);
    } break;

    }
}

static void ncdc_mainwindow_render_status(ncdc_mainwindow_t n)
{
    wchar_t *status = NULL;
    size_t statuslen = 0;
    wchar_t timestr[100] = {0};
    time_t tm = time(NULL);
    struct tm *t = localtime(&tm);
    FILE *f = open_wmemstream(&status, &statuslen);
    wchar_t const *wintitle = NULL;
    ncdc_textview_t view = NULL;
    size_t i = 0;
    dc_channel_t channel = NULL;

    werase(n->sep1);
    return_if_true(f == NULL,);

    wcsftime(timestr, 99, L"[%H:%M]", t);
    fwprintf(f, L"%ls", timestr);

    if (!is_logged_in()) {
        fwprintf(f, L" [not logged in]");
    } else {
        dc_account_t current_account = dc_session_me(current_session);
        fwprintf(f, L" [%s]", dc_account_fullname(current_account));
    }

    view = g_ptr_array_index(n->views, n->curview);
    wintitle = ncdc_textview_title(view);
    fwprintf(f, L" [%d: %ls]", n->curview,
             (wintitle != NULL ? wintitle : L"n/a")
        );

    fwprintf(f, L" [Act:");
    for (i = 0; i < n->views->len; i++) {
        view = g_ptr_array_index(n->views, i);
        channel = ncdc_textview_channel(view);
        if (channel != NULL && dc_channel_has_new_messages(channel)) {
            fwprintf(f, L" %d", i);
        }
    }
    fwprintf(f, L"]");

    fclose(f);
    mvwaddwstr(n->sep1, 0, 0, status);
    free(status);
}

static void ncdc_mainwindow_open_guildchat(ncdc_mainwindow_t n)
{
    ncdc_treeitem_t cur = ncdc_treeview_current(n->guildview);
    dc_channel_t channel = NULL;
    wchar_t *cmd = NULL;

    return_if_true(cur == NULL ||
                   /* not the root, thanks
                    */
                   cur == ncdc_treeview_root(n->guildview) ||
                   /* not one root (again)
                    */
                   ncdc_treeitem_parent(cur) == NULL ||
                   /* not a guild who are the first level after root
                    */
                   ncdc_treeitem_parent(cur) == ncdc_treeview_root(n->guildview),
        );

    channel = ncdc_treeitem_tag(cur);
    return_if_true(channel == NULL,);

    aswprintf(&cmd, L"/join %s", dc_channel_id(channel));
    return_if_true(cmd == NULL,);

    ncdc_dispatch(n, cmd);
    free(cmd);
}

void ncdc_mainwindow_update_guilds(ncdc_mainwindow_t n)
{
    GHashTableIter iter;
    gpointer key = NULL, value = NULL;
    size_t idx = 0;
    GHashTable *parents = NULL;

    ncdc_treeitem_clear(n->root);

    if (!is_logged_in()) {
        return;
    }

    parents = g_hash_table_new(g_str_hash, g_str_equal);

    g_hash_table_iter_init(&iter, dc_session_guilds(current_session));
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        dc_guild_t g = (dc_guild_t)value;
        ncdc_treeitem_t i = ncdc_treeitem_new();
        wchar_t *name = NULL;

        goto_if_true(i == NULL, cleanup);

        name = s_convert(dc_guild_name(g));
        goto_if_true(i ==  NULL, cleanup);

        ncdc_treeitem_set_label(i, name);

        free(name);
        name = NULL;

        ncdc_treeitem_set_tag(i, g);

        /* add subchannels
         */
        for (idx = 0; idx < dc_guild_channels(g); idx++) {
            dc_channel_t c = dc_guild_nth_channel(g, idx);
            ncdc_treeitem_t ci = NULL;

            goto_if_true(dc_channel_name(c) == NULL ||
                         dc_channel_id(c) == NULL, cleanup
                );

            ci = ncdc_treeitem_new();
            goto_if_true(ci == NULL, cleanup);

            if (dc_channel_type(c) == CHANNEL_TYPE_GUILD_VOICE ||
                dc_channel_type(c) == CHANNEL_TYPE_GUILD_TEXT) {
                aswprintf(&name, L"[%s] %s",
                          (dc_channel_type(c) == CHANNEL_TYPE_GUILD_VOICE ?
                           "<" : "#"),
                          dc_channel_name(c)
                    );
            } else {
                aswprintf(&name, L"%s", dc_channel_name(c));
            }

            if (name == NULL) {
                dc_unref(ci);
                continue;
            }

            g_hash_table_insert(parents, (void*)dc_channel_id(c), ci);

            ncdc_treeitem_set_label(ci, name);
            free(name);
            name = NULL;

            ncdc_treeitem_set_tag(ci, c);

            if (dc_channel_parent_id(c) != NULL &&
                g_hash_table_contains(parents, dc_channel_parent_id(c))) {
                ncdc_treeitem_t parent = g_hash_table_lookup(
                    parents, dc_channel_parent_id(c)
                    );
                ncdc_treeitem_add(parent, ci);
            } else {
                ncdc_treeitem_add(i, ci);
            }

            dc_unref(ci);
            ci = NULL;
        }

        ncdc_treeitem_add(n->root, i);

    cleanup:

        dc_unref(i);
        i = NULL;

        free(name);
        name = NULL;
    }

    g_hash_table_unref(parents);
}

void ncdc_mainwindow_input_ready(ncdc_mainwindow_t n)
{
    wint_t i = 0;
    wchar_t *key = NULL;
    size_t keylen = 0;
    ncdc_keybinding_t *k = NULL;

    i = fgetwc(stdin);

    if (i == KEY_ESCAPE) {
        if ((key = util_readkey(i)) == NULL) {
            return;
        }
        keylen = wcslen(key);

#if 0
        FILE *f = fopen("keys.txt", "a+");
        fwprintf(f, L"KEY: %02X %ls\n",
                 key[0], &key[1]
            );
        fclose(f);
#endif
    }

#if 0
    FILE *f = fopen("keys.txt", "a+");
    fwprintf(f, L"%X\n", i);
    fclose(f);
#endif

    if (key != NULL &&
        (k = ncdc_find_keybinding(keys_global, key, keylen)) != NULL) {
        k->handler(n);
        goto cleanup;
    }

    switch (n->focus) {
    case FOCUS_CHAT:
    {
        if (key != NULL &&
            (k = ncdc_find_keybinding(keys_chat, key, keylen)) != NULL) {
            k->handler(n->chat);
        }
    } break;

    case FOCUS_GUILDS:
    {
        if (key != NULL &&
            (k = ncdc_find_keybinding(keys_guilds, key, keylen)) != NULL) {
            k->handler(n->guildview);
        } else if (i == '\r') {
            ncdc_mainwindow_open_guildchat(n);
        }
    } break;

    case FOCUS_INPUT:
    {
        if (n->focus == FOCUS_INPUT) {
            if (key == NULL) {
                ncdc_input_feed(n->in, (wchar_t const *)&i, 1);
            } else {
                ncdc_input_feed(n->in, key, wcslen(key));
            }
        }
    } break;
    }

cleanup:

    free(key);
}

GPtrArray *ncdc_mainwindow_views(ncdc_mainwindow_t n)
{
    return n->views;
}

void ncdc_mainwindow_switchview(ncdc_mainwindow_t n, int idx)
{
    return_if_true(n == NULL || n->views == NULL,);
    return_if_true(idx >= n->views->len,);

    n->curview = idx;
}

void ncdc_mainwindow_switch_view(ncdc_mainwindow_t n, ncdc_textview_t v)
{
    return_if_true(n == NULL || n->views == NULL || v == NULL,);
    guint idx = 0;

    if (g_ptr_array_find(n->views, v, &idx)) {
        n->curview = idx;
    }
}

void ncdc_mainwindow_refresh(ncdc_mainwindow_t n)
{
    ncdc_textview_t v = 0;

    ncdc_treeview_render(n->guildview, n->guilds, n->guilds_h, n->guilds_w);
    wnoutrefresh(n->guilds);

    /* render active text view
     */
    v = g_ptr_array_index(n->views, n->curview);
    ncdc_textview_render(v, n->chat, n->chat_h, n->chat_w);
    wnoutrefresh(n->chat);

    ncdc_input_draw(n->in, n->input);
    wnoutrefresh(n->input);

    wbkgd(n->sep1, COLOR_PAIR(ncdc_colour_separator));
    ncdc_mainwindow_render_status(n);
    wnoutrefresh(n->sep1);

    wbkgd(n->sep2, COLOR_PAIR(ncdc_colour_separator));
    wnoutrefresh(n->sep2);

    ncdc_mainwindow_update_focus(n);
}

void ncdc_mainwindow_log(ncdc_mainwindow_t w, wchar_t const *fmt, ...)
{
    va_list lst;
    wchar_t buf[256] = {0};

    return_if_true(w == NULL || fmt == NULL,);

    va_start(lst, fmt);
    vswprintf(buf, 255, fmt, lst);
    va_end(lst);

    ncdc_textview_append(w->log, buf);
}

ncdc_textview_t
ncdc_mainwindow_switch_or_add(ncdc_mainwindow_t n, dc_channel_t c)
{
    ncdc_textview_t v = NULL;
    wchar_t *name = NULL;

    return_if_true(n == NULL || c == NULL, NULL);
    return_if_true(!is_logged_in(), NULL);

    v = ncdc_mainwindow_channel_view(n, c);
    if (v == NULL) {
        v = ncdc_textview_new();
        if (v == NULL) {
            return NULL;
        }

        ncdc_textview_set_account(v, dc_session_me(current_session));
        ncdc_textview_set_channel(v, c);

        if (dc_channel_type(c) == CHANNEL_TYPE_GUILD_TEXT) {
            aswprintf(&name, L"#%s", dc_channel_name(c));
        } else if (dc_channel_type(c) == CHANNEL_TYPE_GUILD_VOICE) {
            aswprintf(&name, L">%s", dc_channel_name(c));
        } else if (dc_channel_is_dm(c)) {
            size_t namelen = 0, i = 0;
            FILE *f = open_wmemstream(&name, &namelen);

            for (i = 0; i < dc_channel_recipients(c); i++) {
                dc_account_t rec = dc_channel_nth_recipient(c, i);
                if (dc_account_fullname(rec) != NULL) {
                    fwprintf(f, L"%s", dc_account_fullname(rec));
                    if (i < dc_channel_recipients(c)-1) {
                        fputwc('/', f);
                    }
                }
            }

            fclose(f);
        }

        if (name != NULL) {
            ncdc_textview_set_title(v, name);
            free(name);
        }

        g_ptr_array_add(n->views, v);
        ncdc_mainwindow_switch_view(n, v);
    }

    ncdc_mainwindow_switch_view(n, v);
    return v;
}

ncdc_textview_t
ncdc_mainwindow_channel_view(ncdc_mainwindow_t n, dc_channel_t c)
{
    size_t i = 0;

    for (i = 0; i < n->views->len; i++) {
        ncdc_textview_t v = g_ptr_array_index(n->views, i);
        dc_channel_t vc = ncdc_textview_channel(v);

        if (dc_channel_compare(vc, c)) {
            return v;
        }
    }

    return NULL;
}

static void ncdc_mainwindow_ack_view(ncdc_mainwindow_t n)
{
#if 0
    dc_channel_t c = ncdc_mainwindow_current_channel(n);
    return_if_true(c == NULL,);
    return_if_true(dc_channel_messages(c) == 0,);

    dc_message_t m = dc_channel_nth_message(c, dc_channel_messages(c)-1);
    dc_api_channel_ack(dc_session_api(current_session),
                       dc_session_me(current_session),
                       c, m
        );
#endif
}

void ncdc_mainwindow_switch_guilds(ncdc_mainwindow_t n)
{
    return_if_true(n == NULL,);
    n->focus = FOCUS_GUILDS;
    ncdc_mainwindow_update_focus(n);
}

void ncdc_mainwindow_switch_input(ncdc_mainwindow_t n)
{
    return_if_true(n == NULL,);
    n->focus = FOCUS_INPUT;
    ncdc_mainwindow_update_focus(n);
}

void ncdc_mainwindow_switch_chat(ncdc_mainwindow_t n)
{
    return_if_true(n == NULL,);
    n->focus = FOCUS_CHAT;
    ncdc_mainwindow_update_focus(n);
}

void ncdc_mainwindow_rightview(ncdc_mainwindow_t n)
{
    return_if_true(n == NULL,);
    n->curview = (n->curview + 1) % n->views->len;
    ncdc_mainwindow_ack_view(n);
}

void ncdc_mainwindow_leftview(ncdc_mainwindow_t n)
{
    return_if_true(n == NULL,);
    n->curview = (n->curview - 1) % n->views->len;
    ncdc_mainwindow_ack_view(n);
}

dc_channel_t ncdc_mainwindow_current_channel(ncdc_mainwindow_t n)
{
    return_if_true(n == NULL, NULL);
    ncdc_textview_t view = g_ptr_array_index(n->views, n->curview);
    /* can't post to the log channel, that's for internal use only
     */
    return_if_true(view == n->log, NULL);
    return ncdc_textview_channel(view);
}
