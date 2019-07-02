#include <ncdc/ncdc.h>
#include <ncdc/mainwindow.h>
#include <ncdc/config.h>

#include <stdio.h>
#include <unistd.h>

/* event base for libevent
 */
struct event *stdin_ev = NULL;

/* main window
 */
ncdc_mainwindow_t mainwin = NULL;

/* we loop in a different thread
 */
static bool done = false;

/* all the accounts we have logged into
 */
GHashTable *accounts = NULL;

char *ncdc_private_dir = NULL;
void *config = NULL;

dc_loop_t loop = NULL;
dc_api_t api = NULL;

static void ncdc_account_free(void *ptr)
{
    ncdc_account_t a = (ncdc_account_t)ptr;

    return_if_true(ptr == NULL,);

    if (a->friends != NULL) {
        g_ptr_array_unref(a->friends);
    }

    if (a->guilds != NULL) {
        g_ptr_array_unref(a->guilds);
    }

    dc_unref(a->account);
    free(ptr);
}

static void sighandler(int sig)
{
    endwin();
    exit(3);
}

static void cleanup(void)
{
    endwin();

    if (stdin_ev != NULL) {
        event_del(stdin_ev);
        event_free(stdin_ev);
        stdin_ev = NULL;
    }

    done = true;

    if (accounts != NULL) {
        g_hash_table_unref(accounts);
        accounts = NULL;
    }

    dc_unref(api);
    dc_unref(loop);

    dc_unref(config);
    dc_unref(mainwin);
}

static void stdin_handler(int sock, short what, void *data)
{
    if ((what & EV_READ) == EV_READ) {
        ncdc_mainwindow_input_ready(mainwin);
    }
}

static bool init_everything(void)
{
    evthread_use_pthreads();

    setlocale(LC_CTYPE, "");

    loop = dc_loop_new();
    return_if_true(loop == NULL, false);

    /* add handle for STDIN, this info will then be fed to the UI
     */
    stdin_ev = event_new(dc_loop_event_base(loop), 0 /* stdin */,
                         EV_READ|EV_PERSIST,
                         stdin_handler, NULL
        );
    return_if_true(stdin_ev == NULL, false);
    event_add(stdin_ev, NULL);

    /* initialise event
     */
    api = dc_api_new();
    return_if_true(api == NULL, false);

    dc_loop_add_api(loop, api);

    config = ncdc_config_new();
    return_if_true(config == NULL, false);

    accounts = g_hash_table_new_full(g_str_hash, g_str_equal,
                                     g_free, ncdc_account_free
        );
    return_if_true(accounts == NULL, false);

    return true;
}

int main(int ac, char **av)
{
    bool done = false;

    atexit(cleanup);

    signal(SIGINT, sighandler);

    if (getenv("HOME") == NULL) {
        fprintf(stderr, "your environment doesn't contain HOME; pls fix\n");
        return 3;
    }

    asprintf(&ncdc_private_dir, "%s/.ncdc", getenv("HOME"));
    if (mkdir(ncdc_private_dir, 0755) < 0) {
        if (errno != EEXIST) {
            fprintf(stderr, "failed to make %s: %s\n", ncdc_private_dir,
                    strerror(errno));
            return 3;
        }
    }

    if (!init_everything()) {
        return 3;
    }

    done = false;

    initscr();
    noecho();
    nonl();
    keypad(stdscr, TRUE);
    intrflush(NULL, FALSE);

    if (has_colors()) {
        start_color();
        use_default_colors();

        init_pair(1, COLOR_WHITE, COLOR_BLUE);
    }

    mainwin = ncdc_mainwindow_new();
    if (mainwin == NULL) {
        fprintf(stderr, "failed to init ncurses\n");
        return 3;
    }

    while (!done) {
        ncdc_mainwindow_refresh(mainwin);
        doupdate();

        if (!dc_loop_once(loop)) {
            break;
        }
    }

    dc_unref(mainwin);
    endwin();

    return 0;
}
