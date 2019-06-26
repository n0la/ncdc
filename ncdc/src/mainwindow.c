#include <ncdc/mainwindow.h>
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

    char *cmd;
    int cin;
    bool cin_ready;

    int focus;
};

static ncdc_mainwindow_t mainwin = NULL;

static void mainwin_resize(void);
static void mainwin_update_focus(void);
static void mainwin_command(char *s);

static int readline_input_avail(void)
{
    return mainwin->cin_ready;
}

static int readline_getc(FILE *dummy)
{
    mainwin->cin_ready = false;
    return mainwin->cin;
}

static int measure(char const *string)
{
    size_t needed = mbstowcs(NULL, string, 0) + 1;
    wchar_t *wcstring = calloc(needed, sizeof(wchar_t));
    size_t ret = 0;

    return_if_true(wcstring == NULL, -1);

    ret = mbstowcs(wcstring, string, needed);
    if (ret == (size_t)-1) {
        free(wcstring);
        return -1;
    }

    int width = wcswidth(wcstring, needed);
    free(wcstring);

    return width;
}

static void readline_redisplay(void)
{
    int len = 0;
    char *line = NULL;
    int diff = 0;

    asprintf(&line, "%s%s",
             (rl_display_prompt != NULL ? rl_display_prompt : ""),
             (rl_line_buffer != NULL ? rl_line_buffer : "")
        );
    len = measure(line);

    diff = len - mainwin->input_w + 3;
    if (diff > 0) {
        memmove(line, line + diff, len - diff);
        line[len-diff] = '\0';
    }

    werase(mainwin->input);
    mvwprintw(mainwin->input, 1, 1, "%s", line);
    free(line);

    wrefresh(mainwin->input);
}

bool ncdc_mainwindow_init(void)
{
    mainwin = calloc(1, sizeof(struct ncdc_mainwindow_));
    return_if_true(mainwin == NULL, false);

    mainwin->guilds = newwin(5, 5, 1, 1);
    mainwin->chat = newwin(5, 5, 4, 4);
    mainwin->input = newwin(5, 5, 8, 8);
    mainwin_resize();

    mainwin->focus = FOCUS_INPUT;
    mainwin_update_focus();

    rl_getc_function = readline_getc;
    rl_input_available_hook = readline_input_avail;
    rl_redisplay_function = readline_redisplay;
    rl_callback_handler_install("", mainwin_command);

    rl_catch_signals = 0;
    rl_catch_sigwinch = 0;
    rl_deprep_term_function = NULL;
    rl_prep_term_function = NULL;

    return true;
}

static void mainwin_resize(void)
{
    mainwin->guilds_h = LINES;
    mainwin->guilds_w = (COLS / 5);
    mainwin->guilds_y = 0;
    mainwin->guilds_x = 0;

    wresize(mainwin->guilds, mainwin->guilds_h, mainwin->guilds_w);
    mvwin(mainwin->guilds, mainwin->guilds_y, mainwin->guilds_x);
    wnoutrefresh(mainwin->guilds);

    mainwin->input_h = 3;
    mainwin->input_w = COLS - mainwin->guilds_w;
    mainwin->input_y = LINES - mainwin->input_h;
    mainwin->input_x = mainwin->guilds_w;

    wresize(mainwin->input, mainwin->input_h, mainwin->input_w);
    mvwin(mainwin->input, mainwin->input_y, mainwin->input_x);
    wnoutrefresh(mainwin->input);

    mainwin->chat_h = LINES - mainwin->input_h;
    mainwin->chat_w = COLS - mainwin->guilds_w;
    mainwin->chat_y = 0;
    mainwin->chat_x = mainwin->guilds_w;

    wresize(mainwin->chat, mainwin->chat_h, mainwin->chat_w);
    mvwin(mainwin->chat, mainwin->chat_y, mainwin->chat_x);
    wnoutrefresh(mainwin->chat);
}

static void mainwin_command(char *s)
{
    free(mainwin->cmd);
    mainwin->cmd = s;
}

static void mainwin_update_focus(void)
{
    switch (mainwin->focus) {
    case FOCUS_GUILDS:
    {
    } break;

    case FOCUS_CHAT:
    {
    } break;

    case FOCUS_INPUT:
    {
        int x = 1, y = 1;
        wmove(mainwin->input, y, x);
    } break;

    }
}

void ncdc_mainwindow_feed(int ch)
{
    switch (ch) {
    case KEY_RESIZE: mainwin_resize(); break;
    }

    switch (mainwin->focus) {
    case FOCUS_INPUT:
    {
        mainwin->cin = ch;
        mainwin->cin_ready = true;
        rl_callback_read_char();
    } break;

    }
}

void ncdc_mainwindow_refresh(void)
{
    /* move windows
     */
    box(mainwin->guilds, 0, 0);
    box(mainwin->chat, 0, 0);
    box(mainwin->input, 0, 0);

    wrefresh(mainwin->guilds);
    wrefresh(mainwin->chat);
    wrefresh(mainwin->input);

    doupdate();
}
