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

#include <ncdc/ncdc.h>
#include <ncdc/cmds.h>

bool ncdc_cmd_idle(ncdc_mainwindow_t n, size_t ac, wchar_t **av,
                   wchar_t const *f)
{
    bool ret = false;

    return_if_true(!is_logged_in(), false);

    ret = dc_api_set_user_status(dc_session_api(current_session),
                                 dc_session_me(current_session),
                                 DC_API_USER_STATUS_IDLE
        );
    return ret;
}

bool ncdc_cmd_online(ncdc_mainwindow_t n, size_t ac, wchar_t **av,
                     wchar_t const *f)
{
    bool ret = false;

    return_if_true(!is_logged_in(), false);

    ret = dc_api_set_user_status(dc_session_api(current_session),
                                 dc_session_me(current_session),
                                 DC_API_USER_STATUS_ONLINE
        );
    return ret;
}

bool ncdc_cmd_invisible(ncdc_mainwindow_t n, size_t ac, wchar_t **av,
                        wchar_t const *f)
{
    bool ret = false;

    return_if_true(!is_logged_in(), false);

    ret = dc_api_set_user_status(dc_session_api(current_session),
                                 dc_session_me(current_session),
                                 DC_API_USER_STATUS_INVISIBLE
        );
    return ret;
}

bool ncdc_cmd_dnd(ncdc_mainwindow_t n, size_t ac, wchar_t **av,
                  wchar_t const *f)
{
    bool ret = false;

    return_if_true(!is_logged_in(), false);

    ret = dc_api_set_user_status(dc_session_api(current_session),
                                 dc_session_me(current_session),
                                 DC_API_USER_STATUS_DND
        );
    return ret;
}
