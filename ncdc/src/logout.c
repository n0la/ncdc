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

#include <ncdc/cmds.h>
#include <ncdc/ncdc.h>
#include <ncdc/config.h>

bool ncdc_cmd_logout(ncdc_mainwindow_t n, size_t ac,
                     wchar_t **av, wchar_t const *f)
{
    bool ret = false;

    goto_if_true(current_session == NULL ||
                 !dc_session_has_token(current_session), error);

    ret = dc_session_logout(current_session);
    if (!ret) {
        LOG(n, L"logout: failed to log out the current account");
        goto error;
    }

    g_ptr_array_remove(sessions, current_session);

    dc_unref(current_session);
    current_session = NULL;

    ncdc_mainwindow_update_guilds(n);

    LOG(n, L"logout: successfully logged out");

error:

    return ret;
}
