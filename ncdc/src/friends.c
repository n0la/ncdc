#include <ncdc/cmds.h>
#include <ncdc/ncdc.h>

static bool
ncdc_cmd_friends_list(ncdc_mainwindow_t n, size_t ac, wchar_t **av)
{
    bool ret = false;
    size_t i = 0;

    ret = dc_api_get_friends(api, current_account);
    if (!ret) {
        LOG(n, L"friends: list: failed to fetch your friends");
        return false;
    }

    LOG(n, L"/FRIENDS list");
    for (i = 0; i < dc_account_friends_size(current_account); i++) {
        dc_account_t acc = dc_account_nthfriend(current_account, i);
        LOG(n, L"  %s", dc_account_full_username(acc));
    }
    LOG(n, L"End of /FRIENDS list");

    return true;
}

bool ncdc_cmd_friends(ncdc_mainwindow_t n, size_t ac, wchar_t **av)
{
    wchar_t *subcmd = NULL;

    if (ac <= 1) {
        return false;
    }

    if (current_account == NULL ||
        !dc_account_has_token(current_account)) {
        LOG(n, L"friends: not logged in");
        return false;
    }

    subcmd = av[1];

    --ac;
    ++av;

    if (wcscmp(subcmd, L"list") == 0) {
        return ncdc_cmd_friends_list(n, ac, av);
    } else {
        return false;
    }

    return true;
}
