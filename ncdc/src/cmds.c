#include <ncdc/cmds.h>

ncdc_commands_t cmds[] = {
    { L"quit", ncdc_cmd_quit },
    { NULL, NULL }
};

bool ncdc_cmd_quit(ncdc_mainwindow_t n, wchar_t const *s, size_t len)
{
    exit(0);
}
