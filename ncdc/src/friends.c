#include <ncdc/cmds.h>
#include <ncdc/ncdc.h>

static bool
ncdc_cmd_friends_list(ncdc_mainwindow_t n, size_t ac,
                      wchar_t **av, wchar_t const *f)
{
    size_t i = 0;
    char c = ' ';
    dc_account_t current_account = dc_session_me(current_session);

    LOG(n, L"/FRIENDS list");
    for (i = 0; i < dc_account_friends_size(current_account); i++) {
        dc_account_t acc = dc_account_nth_friend(current_account, i);
        char const *status = dc_account_status(acc);

        if (status == NULL) {
            status = "offline";
        }

        switch (dc_account_friend_state(acc)) {
        case FRIEND_STATE_FRIEND:  c = 'F'; break;
        case FRIEND_STATE_PENDING: c = 'P'; break;
        default: c = ' '; break;
        }

        LOG(n, L" [%c] [%-7s] %s", c, status, dc_account_fullname(acc));
    }
    LOG(n, L"End of /FRIENDS list");

    return true;
}

static bool
ncdc_cmd_friends_add(ncdc_mainwindow_t n, size_t ac,
                     wchar_t **av, wchar_t const *f)
{
    char *name = NULL;
    dc_account_t friend = NULL;
    bool ret = false;
    dc_account_t current_account = dc_session_me(current_session);

    if (ac <= 1) {
        return false;
    }

    name = w_convert(av[1]);
    return_if_true(name == NULL, false);

    friend = dc_account_from_fullname(name);
    if (friend == NULL) {
        LOG(n, L"friends: add: invalid username given, use the full ID");
        goto cleanup;
    }

    if (!dc_api_add_friend(api, current_account, friend)) {
        LOG(n, L"friends: add: failed to add friend, Vulkan would be sad");
        goto cleanup;
    }

    LOG(n, L"friends: add: request for friendship sent to %s", name);
    ret = true;

cleanup:

    dc_unref(friend);
    free(name);

    return ret;
}

static bool
ncdc_cmd_friends_remove(ncdc_mainwindow_t n, size_t ac,
                        wchar_t **av, wchar_t const *f)
{
    char *name = NULL;
    dc_account_t friend = NULL;
    bool ret = false;
    size_t i = 0;
    dc_account_t current_account = dc_session_me(current_session);

    if (ac <= 1) {
        return false;
    }

    name = w_convert(av[1]);
    return_if_true(name == NULL, false);

    for (i = 0; i < dc_account_friends_size(current_account); i++) {
        dc_account_t cur = dc_account_nth_friend(current_account, i);
        if (strcmp(dc_account_fullname(cur), name) == 0) {
            friend = cur;
            break;
        }
    }

    if (friend == NULL) {
        LOG(n, L"friends: remove: no such friend in friend's list");
        goto cleanup;
    }

    if (!dc_api_remove_friend(api, current_account, friend)) {
        LOG(n, L"friends: remove: failed to remove friend");
        goto cleanup;
    }

    LOG(n, L"friends: remove: friend %s removed", name);
    ret = true;

cleanup:

    free(name);

    return ret;
}

static bool
ncdc_cmd_friends_accept(ncdc_mainwindow_t n, size_t ac,
                        wchar_t **av, wchar_t const *f)
{
    char *name = NULL;
    dc_account_t friend = NULL;
    bool ret = false;
    size_t i = 0;
    dc_account_t current_account = dc_session_me(current_session);

    if (ac <= 1) {
        return false;
    }

    name = w_convert(av[1]);
    return_if_true(name == NULL, false);

    for (i = 0; i < dc_account_friends_size(current_account); i++) {
        dc_account_t cur = dc_account_nth_friend(current_account, i);
        if (strcmp(dc_account_fullname(cur), name) == 0 &&
            dc_account_friend_state(cur) == FRIEND_STATE_PENDING) {
            friend = cur;
            break;
        }
    }

    if (friend == NULL) {
        LOG(n, L"friends: accept: no such pending friend on the list");
        goto cleanup;
    }

    if (!dc_api_accept_friend(api, current_account, friend)) {
        LOG(n, L"friends: accept: failed to accept friend");
        goto cleanup;
    }

    LOG(n, L"friends: accept: friend %s accepted", name);
    ret = true;

cleanup:

    free(name);

    return ret;
}

static ncdc_commands_t subcmds[] = {
    { L"accept", ncdc_cmd_friends_accept },
    { L"add",    ncdc_cmd_friends_add },
    { L"list",   ncdc_cmd_friends_list },
    { L"remove", ncdc_cmd_friends_remove },
    { NULL,     NULL }
};

bool ncdc_cmd_friends(ncdc_mainwindow_t n, size_t ac,
                      wchar_t **av, wchar_t const *f)
{
    wchar_t *subcmd = NULL;
    ncdc_commands_t *it = NULL;

    if (!is_logged_in()) {
        LOG(n, L"friends: not logged in");
        return false;
    }

    if (ac <= 1) {
        return ncdc_cmd_friends_list(n, ac, av, f);
    }

    subcmd = av[1];

    --ac;
    ++av;

    if ((it = ncdc_find_cmd(subcmds, subcmd)) == NULL) {
        LOG(n, L"friends: no such subcommand \"%ls\"", subcmd);
        return false;
    }

    return it->handler(n, ac, av, f);
}
