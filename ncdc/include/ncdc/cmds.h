#ifndef NCDC_CMDS_H
#define NCDC_CMDS_H

#include <ncdc/ncdc.h>
#include <ncdc/mainwindow.h>

/**
 * n      .. the main window handle
 * ac,av  .. the full string parsed up along white spaces
 * f      .. the full string, without the /command bit
 */
typedef bool (*ncdc_command_t)(ncdc_mainwindow_t n,
                               size_t argc, wchar_t **argv,
                               wchar_t const *f);

typedef struct {
    wchar_t const *name;
    ncdc_command_t handler;
} ncdc_commands_t;

extern ncdc_commands_t cmds[];

bool ncdc_dispatch_init(void);
bool ncdc_dispatch_deinit(void);

bool ncdc_dispatch(ncdc_mainwindow_t n, wchar_t const *s);

/* find a given command in a list of commands, helpful if your command has
 * sub commands. for example usage see the friends command
 */
ncdc_commands_t *ncdc_find_cmd(ncdc_commands_t *cmds, wchar_t const *name);

bool ncdc_cmd_ack(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f);
bool ncdc_cmd_friends(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f);
bool ncdc_cmd_login(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f);
bool ncdc_cmd_logout(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f);
bool ncdc_cmd_msg(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f);
bool ncdc_cmd_post(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f);
bool ncdc_cmd_quit(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f);

#endif
