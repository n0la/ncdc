#include <ncdc/cmds.h>
#include <ncdc/ncdc.h>
#include <ncdc/config.h>

bool ncdc_cmd_logout(ncdc_mainwindow_t n, size_t ac,
                     wchar_t **av, wchar_t const *f)
{
    bool ret = false;

    goto_if_true(current_session == NULL ||
                 !dc_session_has_token(current_session), error);

    ret = dc_session_logout(current_session);
    if (!ret) {
        LOG(n, L"logout: failed to log out the current account");
        goto error;
    }

    g_ptr_array_remove(sessions, current_session);

    dc_unref(current_session);
    current_session = NULL;

    ncdc_mainwindow_update_guilds(n);

    LOG(n, L"logout: successfully logged out");

error:

    return ret;
}
