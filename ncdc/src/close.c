#include <ncdc/cmds.h>
#include <ncdc/ncdc.h>

bool ncdc_cmd_close(ncdc_mainwindow_t n, size_t ac, wchar_t **av,
                    wchar_t const *f)
{
    ncdc_mainwindow_close_view(n, -1);
    return true;
}
