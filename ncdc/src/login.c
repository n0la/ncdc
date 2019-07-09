#include <ncdc/cmds.h>
#include <ncdc/ncdc.h>
#include <ncdc/config.h>

bool ncdc_cmd_login(ncdc_mainwindow_t n, size_t ac, wchar_t **av)
{
    char *arg = NULL;
    bool ret = false;
    dc_account_t acc = NULL;
    dc_gateway_t gw = NULL;

    goto_if_true(ac <= 1, cleanup);

    arg = w_convert(av[1]);
    goto_if_true(arg == NULL, cleanup);

    acc = g_hash_table_lookup(accounts, arg);
    if (acc == NULL) {
        acc = ncdc_config_account(config, arg);
        if (acc == NULL) {
            LOG(n, L"login: %ls: no such account in configuration", av[1]);
            goto cleanup;
        }

        g_hash_table_insert(accounts, strdup(arg), acc);
    } else {
        if (dc_account_has_token(acc)) {
            LOG(n, L"login: %ls: this account is already logged in", av[1]);
            goto cleanup;
        }
    }

    if (!dc_api_login(api, acc)) {
        LOG(n, L"login: %ls: authentication failed; wrong password?", av[1]);
        goto cleanup;
    }

    gw = dc_gateway_new();
    if (gw == NULL) {
        LOG(n, L"login: %ls: failed to establish gateway", av[1]);
        goto cleanup;
    }

    dc_gateway_set_login(gw, acc);
    dc_loop_add_gateway(loop, gw);

    dc_unref(current_account);
    current_account = dc_ref(acc);

    LOG(n, L"login: %ls: authentication successful", av[1]);
    ret = true;

cleanup:

    free(arg);

    return ret;
}
