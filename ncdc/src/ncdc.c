#include <ncdc/ncdc.h>
#include <dc/api.h>

#include <stdio.h>
#include <unistd.h>

/* event base for libevent
 */
struct event_base *base = NULL;
struct event *stdin_ev = NULL;
struct event *timer = NULL;

/* we loop in a different thread
 */
static bool done = false;
static pthread_t looper;

char *dc_private_dir = NULL;
char *dc_config_file = NULL;

static GKeyFile *config = NULL;

/* global curl multi for API access
 */
CURLM *curl = NULL;

dc_api_t api = NULL;

static void handle_multi_info(void);

static void sighandler(int sig)
{
    exit(3);
}

static void cleanup(void)
{
    if (stdin_ev != NULL) {
        event_del(stdin_ev);
        event_free(stdin_ev);
        stdin_ev = NULL;
    }

    if (timer != NULL) {
        evtimer_del(timer);
        event_free(timer);
        timer = NULL;
    }

    done = true;
    pthread_join(looper, NULL);

    curl_multi_cleanup(curl);
    curl = NULL;

    event_base_free(base);
    base = NULL;
}

static void stdin_handler(int sock, short what, void *data)
{
}

static void mcurl_socket_handler(int sock, short what, void *data)
{
    int unused = 0;

    if ((what & EV_READ) == EV_READ) {
        curl_multi_socket_action(curl, sock, CURL_CSELECT_IN, &unused);
    } else if ((what & EV_WRITE) == EV_WRITE) {
        curl_multi_socket_action(curl, sock, CURL_CSELECT_OUT, &unused);
    }
}

static void timer_handler(int sock, short what, void *data)
{
    int running = 0;
    curl_multi_socket_action(curl, CURL_SOCKET_TIMEOUT, 0, &running);
}

static int mcurl_timer(CURLM *curl, long timeout, void *ptr)
{
    int running = 0;
    struct timeval tm;

    if (timeout == -1) {
        evtimer_del(timer);
    } else if (timeout == 0) {
        curl_multi_socket_action(curl, CURL_SOCKET_TIMEOUT, 0, &running);
    } else if (timeout > 0) {
        tm.tv_sec = timeout / 1000;
        tm.tv_usec = (timeout % 1000) * 1000;
        evtimer_add(timer, &tm);
    }

    return 0;
}

static int
mcurl_handler(CURL *easy, curl_socket_t s, int what, void *userp, void *socketp)
{
    struct event *event = (struct event *)socketp;

    if (what == CURL_POLL_REMOVE) {
        if (event != NULL) {
            event_del(event);
            event_free(event);
            curl_multi_assign(curl, s, NULL);
        }
    } else {
        int stat =
            ((what & CURL_POLL_IN) ? EV_READ : 0) |
            ((what & CURL_POLL_OUT) ? EV_WRITE : 0) |
            EV_PERSIST
            ;

        if (event == NULL) {
            event = event_new(base, s, stat, mcurl_socket_handler, NULL);
            if (event == NULL) {
                return 0;
            }
            curl_multi_assign(curl, s, event);
        } else {
            event_del(event);
            event_assign(event, base, s, stat, mcurl_socket_handler, NULL);
            event_add(event, NULL);
        }
    }

    return 0;
}

static bool init_everything(void)
{
    evthread_use_pthreads();

    base = event_base_new();
    return_if_true(base == NULL, false);

    /* add handle for STDIN, this info will then be fed to the UI
     */
    stdin_ev = event_new(base, 0 /* stdin */, EV_READ|EV_PERSIST,
                         stdin_handler, NULL
        );
    return_if_true(stdin_ev == NULL, false);
    event_add(stdin_ev, NULL);

    timer = evtimer_new(base, timer_handler, NULL);
    return_if_true(timer == NULL, false);

    /* create curl multi and feed that to the API too
     */
    curl = curl_multi_init();
    return_if_true(curl == NULL, false);

    curl_multi_setopt(curl, CURLMOPT_SOCKETFUNCTION, mcurl_handler);
    curl_multi_setopt(curl, CURLMOPT_TIMERFUNCTION, mcurl_timer);

    /* initialise event
     */
    api = dc_api_new();
    return_if_true(api == NULL, false);

    dc_api_set_event_base(api, base);
    dc_api_set_curl_multi(api, curl);

    config = g_key_file_new();
    return_if_true(config == NULL, false);

    g_key_file_load_from_file(config, dc_config_file, 0, NULL);

    return true;
}

static void handle_multi_info(void)
{
    struct CURLMsg *msg = NULL;
    int remain = 0;

    /* check for finished multi curls
     */
    msg = curl_multi_info_read(curl, &remain);
    if (msg != NULL) {
        if (remain <= 0) {
            if (evtimer_pending(timer, NULL)) {
                evtimer_del(timer);
            }
        }
        if (msg->msg == CURLMSG_DONE) {
            dc_api_signal(api, msg->easy_handle, msg->data.result);
        }
    } else {
        usleep(10 * 1000);
    }
}

static void *loop_thread(void *arg)
{
    int ret = 0;

    while (!done) {
        ret = event_base_loop(base, EVLOOP_ONCE|EVLOOP_NONBLOCK);
        if (ret < 0) {
            break;
        }

        handle_multi_info();
    }

    return NULL;
}

static dc_account_t account_from_config(void)
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
    if (pthread_create(&looper, NULL, loop_thread, &done)) {
        return 3;
    }

    dc_account_t a = account_from_config();
    if (a == NULL) {
        fprintf(stderr, "no account specified in config file; sho-sho!\n");
        return 3;
    }

    if (!dc_api_authenticate(api, a)) {
        fprintf(stderr, "authentication failed, wrong password?\n");
        return 3;
    }

    if (!dc_api_userinfo(api, a, a)) {
        fprintf(stderr, "failed to get user information\n");
        return 3;
    }

    return 0;
}
