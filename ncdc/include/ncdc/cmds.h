/*
 * Part of ncdc - a discord client for the console
 * Copyright (C) 2019 Florian Stinglmayr <fstinglmayr@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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
wchar_t **ncdc_cmd_names(void);
size_t ncdc_cmd_names_size(void);

bool ncdc_cmd_ack(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f);
bool ncdc_cmd_close(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f);
bool ncdc_cmd_dnd(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f);
bool ncdc_cmd_friends(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f);
bool ncdc_cmd_idle(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f);
bool ncdc_cmd_invisible(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f);
bool ncdc_cmd_join(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f);
bool ncdc_cmd_login(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f);
bool ncdc_cmd_logout(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f);
bool ncdc_cmd_msg(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f);
bool ncdc_cmd_online(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f);
bool ncdc_cmd_post(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f);
bool ncdc_cmd_quit(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f);

#endif
