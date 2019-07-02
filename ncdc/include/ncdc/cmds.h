#ifndef NCDC_CMDS_H
#define NCDC_CMDS_H

#include <ncdc/ncdc.h>
#include <ncdc/mainwindow.h>

typedef bool (*ncdc_command_t)(ncdc_mainwindow_t n,
                               size_t argc, wchar_t **argv);

typedef struct {
    wchar_t const *name;
    ncdc_command_t handler;
} ncdc_commands_t;

extern ncdc_commands_t cmds[];

bool ncdc_dispatch(ncdc_mainwindow_t n, wchar_t const *s);

bool ncdc_cmd_quit(ncdc_mainwindow_t n, size_t ac, wchar_t **av);
bool ncdc_cmd_login(ncdc_mainwindow_t n, size_t ac, wchar_t **av);

#endif
