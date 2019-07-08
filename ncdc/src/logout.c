#include <ncdc/cmds.h>
#include <ncdc/ncdc.h>
#include <ncdc/config.h>

bool ncdc_cmd_logout(ncdc_mainwindow_t n, size_t ac, wchar_t **av)
{
    bool ret = false;

    goto_if_true(current_account == NULL ||
                 !dc_account_has_token(current_account), error);

    ret = dc_api_logout(api, current_account);
    if (!ret) {
        LOG(n, L"logout: failed to log out the current account");
        goto error;
    }

    g_hash_table_remove(accounts, dc_account_fullname(current_account));

    dc_unref(current_account);
    current_account = NULL;

    LOG(n, L"logout: successfully logged out");

error:

    return ret;
}
