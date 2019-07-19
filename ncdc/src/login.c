#include <ncdc/cmds.h>
#include <ncdc/ncdc.h>
#include <ncdc/config.h>

bool ncdc_cmd_login(ncdc_mainwindow_t n, size_t ac,
                    wchar_t **av, wchar_t const *f)
{
    char *arg = NULL;
    bool ret = false;
    dc_account_t acc = NULL;
    dc_session_t s = NULL;
    uint32_t idx = 0;

    goto_if_true(ac <= 1, cleanup);

    arg = w_convert(av[1]);
    goto_if_true(arg == NULL, cleanup);

    acc = ncdc_config_account(config, arg);
    if (acc == NULL) {
        LOG(n, L"login: %ls: no such account in configuration", av[1]);
        goto cleanup;
    }

    ret = g_ptr_array_find_with_equal_func(
        sessions, arg,
        (GEqualFunc)dc_session_equal_me_fullname,
        &idx
        );
    if (!ret) {
        s = dc_session_new(loop);
        if (s == NULL) {
            goto cleanup;
        }

        g_ptr_array_add(sessions, s);
    } else {
        s = g_ptr_array_index(sessions, idx);
        if (dc_session_has_token(s)) {
            LOG(n, L"login: %ls: this account is already logged in", av[1]);
            goto cleanup;
        }
    }

    if (!dc_session_login(s, acc)) {
        LOG(n, L"login: %ls: authentication failed; wrong password?", av[1]);
        goto cleanup;
    }

    dc_unref(current_session);
    current_session = dc_ref(s);

    LOG(n, L"login: %ls: authentication successful, waiting for ready from websocket...",
        av[1]
        );

    while (!dc_session_is_ready(current_session))
        ;

    LOG(n, L"login: %ls: ready", av[1]);
    ncdc_mainwindow_update_guilds(n);

    ret = true;

cleanup:

    dc_unref(acc);
    free(arg);

    return ret;
}
