#include <ncdc/cmds.h>
#include <ncdc/ncdc.h>
#include <ncdc/config.h>

bool ncdc_cmd_login(ncdc_mainwindow_t n, size_t ac, wchar_t **av)
{
    char *arg = NULL;
    bool ret = false;
    ncdc_account_t ptr = NULL;
    dc_account_t acc = NULL;

    goto_if_true(ac <= 1, cleanup);

    arg = w_convert(av[1]);
    goto_if_true(arg == NULL, cleanup);

    if (g_hash_table_lookup(accounts, arg) != NULL) {
        LOG(n, L"login: %ls: this account is already logged in", av[1]);
        goto cleanup;
    }

    acc = ncdc_config_account(config, arg);
    if (acc == NULL) {
        LOG(n, L"login: %ls: no such account in configuration", av[1]);
        goto cleanup;
    }

    ptr = calloc(1, sizeof(struct ncdc_account_));
    goto_if_true(ptr == NULL, cleanup);

    ptr->account = acc;
    g_hash_table_insert(accounts, arg, ptr);

    ret = true;

cleanup:

    free(arg);

    return ret;
}
