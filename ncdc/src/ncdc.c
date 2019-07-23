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

#include <ncdc/ncdc.h>
#include <ncdc/mainwindow.h>
#include <ncdc/config.h>
#include <ncdc/cmds.h>

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
bool main_done = false;
bool thread_done = false;
static pthread_t event_thread;
static struct event_base *base = NULL;

/* all the sessions we currently have around
 */
GPtrArray *sessions = NULL;
dc_session_t current_session = NULL;

char *ncdc_private_dir = NULL;
void *config = NULL;

dc_loop_t loop = NULL;

/* API handle we use for everything
 */
dc_api_t api = NULL;

static void cleanup(void)
{
    endwin();

    if (sessions != NULL) {
        g_ptr_array_unref(sessions);
        sessions = NULL;
    }

    dc_unref(current_session);
    current_session = NULL;

    thread_done = true;
    dc_loop_abort(loop);
    pthread_join(event_thread, NULL);

    if (stdin_ev != NULL) {
        event_del(stdin_ev);
        event_free(stdin_ev);
        stdin_ev = NULL;
    }

    event_base_loopbreak(base);
    event_base_free(base);
    base = NULL;

    dc_unref(api);
    dc_unref(loop);

    dc_unref(config);
    dc_unref(mainwin);
}

static void handle_winch(int sig)
{
    /* according to ncurses documentation you should call
     * endwin(), followed by a refresh() to properly update
     * curses
     */
    endwin();
    refresh();
    clear();

    ncdc_mainwindow_resize(mainwin);
}

static void sighandler(int sig)
{
    exit_main();
    exit(3);
}

static void stdin_handler(int sock, short what, void *data)
{
    if ((what & EV_READ) == EV_READ) {
        ncdc_mainwindow_input_ready(mainwin);
    }
}

static void *looper(void *arg)
{
    while (!thread_done) {
        if (!dc_loop_once(loop)) {
            break;
        }

        usleep(10 * 1000);
    }

    return NULL;
}

static bool init_everything(void)
{
    int ret = 0;

    evthread_use_pthreads();

    setlocale(LC_CTYPE, "");

    return_if_true(!ncdc_dispatch_init(), false);

    base = event_base_new();
    return_if_true(base == NULL, false);

    loop = dc_loop_new();
    return_if_true(loop == NULL, false);

    /* add handle for STDIN, this info will then be fed to the UI
     */
    stdin_ev = event_new(base, 0 /* stdin */,
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

    sessions = g_ptr_array_new_with_free_func((GDestroyNotify)dc_unref);
    return_if_true(sessions == NULL, false);

    ret = pthread_create(&event_thread, NULL, looper, NULL);
    return_if_true(ret != 0, false);

    return true;
}

void exit_main(void)
{
    main_done = true;
    event_base_loopbreak(base);
}

int main(int ac, char **av)
{
    int ret = 0;

    signal(SIGINT, sighandler);
    signal(SIGWINCH, handle_winch);

    if (getenv("HOME") == NULL) {
        fprintf(stderr, "your environment doesn't contain HOME; pls fix\n");
        return 3;
    }

    asprintf(&ncdc_private_dir, "%s/.config/ncdc", getenv("HOME"));
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

    initscr();
    cbreak();
    noecho();
    nonl();
    keypad(stdscr, TRUE);
    intrflush(NULL, FALSE);

    if (has_colors()) {
        start_color();
        use_default_colors();

        init_pair(ncdc_colour_separator, COLOR_WHITE, COLOR_BLUE);
        init_pair(ncdc_colour_treehighlight, COLOR_WHITE, COLOR_RED);
    }

    mainwin = ncdc_mainwindow_new();
    if (mainwin == NULL) {
        fprintf(stderr, "failed to init ncurses\n");
        return 3;
    }

    while (!main_done) {
        ncdc_mainwindow_refresh(mainwin);
        doupdate();

        ret = event_base_loop(base, EVLOOP_ONCE|EVLOOP_NONBLOCK);
        if (ret < 0) {
            break;
        }

        usleep(10*1000);
    }

    cleanup();

    return 0;
}
