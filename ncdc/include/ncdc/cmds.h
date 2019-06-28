#ifndef NCDC_CMDS_H
#define NCDC_CMDS_H

#include <ncdc/ncdc.h>
#include <ncdc/mainwindow.h>

typedef bool (*ncdc_command_t)(ncdc_mainwindow_t n,
                               wchar_t const *s, size_t len);

typedef struct {
    wchar_t const *name;
    ncdc_command_t handler;
} ncdc_commands_t;

extern ncdc_commands_t cmds[];

bool ncdc_cmd_quit(ncdc_mainwindow_t n, wchar_t const *s, size_t len);

#endif
