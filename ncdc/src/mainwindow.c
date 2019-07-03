#include <ncdc/mainwindow.h>
#include <ncdc/input.h>
#include <ncdc/textview.h>
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
    dc_unref(n->log);

    free(n);
}

ncdc_mainwindow_t ncdc_mainwindow_new(void)
{
    ncdc_mainwindow_t ptr = calloc(1, sizeof(struct ncdc_mainwindow_));
    return_if_true(ptr == NULL, NULL);

    ptr->ref.cleanup = (dc_cleanup_t)ncdc_mainwindow_free;

    ptr->in = ncdc_input_new();
    ncdc_input_set_callback(ptr->in, ncdc_mainwindow_callback, ptr);

    ptr->log = ncdc_textview_new();

    ptr->guilds = newwin(5, 5, 1, 1);
    ptr->chat = newwin(5, 5, 4, 4);

    ptr->input = newwin(5, 5, 8, 8);
    keypad(ptr->input, TRUE);

    ptr->sep1 = newwin(5, 5, 10, 10);
    ptr->sep2 = newwin(5, 5, 12, 12);

    ncdc_mainwindow_resize(ptr);

    ptr->focus = FOCUS_INPUT;
    ncdc_mainwindow_update_focus(ptr);

    return ptr;
}

static bool
ncdc_mainwindow_callback(ncdc_input_t i, wchar_t const *s,
                         size_t len, void *arg)
{
    ncdc_mainwindow_t mainwin = (ncdc_mainwindow_t)arg;

    if (s != NULL && s[0] == '/') {
        if (s[1] == '\0') {
            return false;
        }

        return ncdc_dispatch(mainwin, s);
    }

    return false;
}

static void ncdc_mainwindow_resize(ncdc_mainwindow_t n)
{
    n->guilds_h = LINES - 2;
    n->guilds_w = (COLS / 5);
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
    } break;

    case FOCUS_CHAT:
    {
    } break;

    case FOCUS_INPUT:
    {
        wmove(n->input, 0, ncdc_input_cursor(n->in));
        wrefresh(n->input);
    } break;

    }
}

void ncdc_mainwindow_input_ready(ncdc_mainwindow_t n)
{
    switch (n->focus) {
    case FOCUS_INPUT:
    {
        wint_t i = 0;

        if (wget_wch(n->input, &i) == ERR) {
            return;
        }

        if (i == KEY_RESIZE) {
            ncdc_mainwindow_resize(n);
        } else {
            ncdc_input_feed(n->in, (wchar_t)i);
        }
    } break;

    }
}

void ncdc_mainwindow_refresh(ncdc_mainwindow_t n)
{
    wnoutrefresh(n->guilds);

    ncdc_textview_render(n->log, n->chat, n->chat_h, n->chat_w);
    wnoutrefresh(n->chat);

    ncdc_input_draw(n->in, n->input);
    wnoutrefresh(n->input);

    wbkgd(n->sep1, COLOR_PAIR(1));
    wnoutrefresh(n->sep1);
    wbkgd(n->sep2, COLOR_PAIR(1));
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
