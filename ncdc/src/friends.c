#include <ncdc/cmds.h>
#include <ncdc/ncdc.h>

static bool
ncdc_cmd_friends_list(ncdc_mainwindow_t n, size_t ac, wchar_t **av)
{
    bool ret = false;
    size_t i = 0;
    wchar_t c = ' ';

    ret = dc_api_get_friends(api, current_account);
    if (!ret) {
        LOG(n, L"friends: list: failed to fetch your friends");
        return false;
    }

    LOG(n, L"/FRIENDS list");
    for (i = 0; i < dc_account_friends_size(current_account); i++) {
        dc_account_t acc = dc_account_nthfriend(current_account, i);
        switch (dc_account_friend_state(acc)) {
        case FRIEND_STATE_PENDING: c = 'P'; break;
        default: c = ' '; break;
        }
        LOG(n, L"%lc %s", c, dc_account_full_username(acc));
    }
    LOG(n, L"End of /FRIENDS list");

    return true;
}

static bool
ncdc_cmd_friends_add(ncdc_mainwindow_t n, size_t ac, wchar_t **av)
{
    char *name = NULL;
    dc_account_t friend = NULL;
    bool ret = false;

    if (ac <= 1) {
        return false;
    }

    name = w_convert(av[1]);
    return_if_true(name == NULL, false);

    friend = dc_account_from_fullid(name);
    if (friend == NULL) {
        LOG(n, L"friends: add: invalid username given, use the full ID");
        goto cleanup;
    }

    if (!dc_api_add_friend(api, current_account, friend)) {
        LOG(n, L"friends: add: failed to add friend, Vulkan would be sad");
        goto cleanup;
    }

    LOG(n, L"friends: add: request for friendship sent");
    ret = true;

cleanup:

    dc_unref(friend);
    free(name);

    return ret;
}

bool ncdc_cmd_friends(ncdc_mainwindow_t n, size_t ac, wchar_t **av)
{
    wchar_t *subcmd = NULL;

    if (current_account == NULL ||
        !dc_account_has_token(current_account)) {
        LOG(n, L"friends: not logged in");
        return false;
    }

    if (ac <= 1) {
        return ncdc_cmd_friends_list(n, ac, av);
    }

    subcmd = av[1];

    --ac;
    ++av;

    if (wcscmp(subcmd, L"list") == 0) {
        return ncdc_cmd_friends_list(n, ac, av);
    } else if (wcscmp(subcmd, L"add") == 0) {
        return ncdc_cmd_friends_add(n, ac, av);
    } else {
        return false;
    }

    return true;
}
