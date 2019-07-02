#include <ncdc/cmds.h>

ncdc_commands_t cmds[] = {
    { L"login", ncdc_cmd_login },
    { L"quit", ncdc_cmd_quit },
    { NULL, NULL }
};

bool ncdc_dispatch(ncdc_mainwindow_t n, wchar_t const *s)
{
    wchar_t **tokens = NULL;
    size_t size = 0;
    size_t i = 0;
    bool ret = false;

    tokens = w_tokenise(s);
    return_if_true(tokens == NULL, false);

    size = w_strlenv(tokens);

    for (i = 0; cmds[i].name != NULL; i++) {
        if (wcscmp(cmds[i].name, tokens[0]) == 0) {
            ret = cmds[i].handler(n, size, tokens);
            break;
        }
    }

    w_strfreev(tokens);

    return ret;
}

bool ncdc_cmd_quit(ncdc_mainwindow_t n, size_t ac, wchar_t **av)
{
    exit(0);
}
