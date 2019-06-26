#include <ncdc/ncdc.h>
#include <ncdc/mainwindow.h>

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

char *dc_private_dir = NULL;
char *dc_config_file = NULL;

static GKeyFile *config = NULL;

dc_loop_t loop = NULL;
dc_api_t api = NULL;

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

    dc_unref(api);
    dc_unref(loop);
}

static void stdin_handler(int sock, short what, void *data)
{
    int ch = 0;

    if ((what & EV_READ) == EV_READ) {
        ch = getch();
        ncdc_mainwindow_feed(ch);
    }
}

static bool init_everything(void)
{
    evthread_use_pthreads();

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

    config = g_key_file_new();
    return_if_true(config == NULL, false);

    g_key_file_load_from_file(config, dc_config_file, 0, NULL);

    return true;
}

dc_account_t account_from_config(void)
{
    char const *email = NULL;
    char const *password = NULL;
    void *ptr = NULL;

    email = g_key_file_get_string(config, "account", "email", NULL);
    password = g_key_file_get_string(config, "account", "password", NULL);

    return_if_true(email == NULL || password == NULL, NULL);

    ptr = dc_account_new2(email, password);
    dc_account_set_id(ptr, "@me");

    return ptr;
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

    asprintf(&dc_private_dir, "%s/.ndc", getenv("HOME"));
    if (mkdir(dc_private_dir, 0755) < 0) {
        if (errno != EEXIST) {
            fprintf(stderr, "failed to make %s: %s\n", dc_private_dir,
                    strerror(errno));
            return 3;
        }
    }

    asprintf(&dc_config_file, "%s/config", dc_private_dir);

    if (!init_everything()) {
        return 3;
    }

    done = false;

    initscr();
    cbreak();
    noecho();
    nonl();
    intrflush(NULL, FALSE);

    if (has_colors()) {
        start_color();
        use_default_colors();
    }

    if (!ncdc_mainwindow_init()) {
        fprintf(stderr, "failed to init ncurses\n");
        return 3;
    }

    while (!done) {
        ncdc_mainwindow_refresh();

        if (!dc_loop_once(loop)) {
            break;
        }
    }

    endwin();

    return 0;
}
