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

bool ncdc_cmd_ack(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f)
{
    dc_channel_t c = NULL;
    dc_message_t m = NULL;
    bool ret = false;

    if (!is_logged_in()) {
        return false;
    }

    c = ncdc_mainwindow_current_channel(n);
    return_if_true(c == NULL, false);
    return_if_true(dc_channel_messages(c) == 0, false);
    m = dc_channel_nth_message(c, dc_channel_messages(c)-1);


    ret = dc_api_channel_ack(dc_session_api(current_session),
                             dc_session_me(current_session),
                             c, m
        );

    if (!ret) {
        LOG(n, L"ack: failed to ack the given channel");
        return false;
    }

    dc_channel_mark_read(c);

    return true;
}
