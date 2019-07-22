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

bool
ncdc_cmd_join(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f)
{
    char *guild = NULL;
    char *channel = NULL;
    char *id = NULL;
    bool ret = true;
    dc_guild_t g = NULL;
    dc_channel_t c = NULL;

    if (ac <= 1) {
        LOG(n, L"join: not enough arguments given");
        return false;
    }

    if (!is_logged_in()) {
        return false;
    }

    if (ac == 3) {
        guild = w_convert(av[1]);
        channel = w_convert(av[2]);

        g = dc_session_guild_by_name(current_session, guild);
        if (g == NULL) {
            LOG(n, L"join: no such guild: %s", guild);
            goto cleanup;
        }

        c = dc_guild_channel_by_name(g, channel);
        if (c == NULL) {
            LOG(n, L"join: no such channel %s in guild %s", channel, guild);
            goto cleanup;
        }
    } else if (ac == 2) {
        id = w_convert(av[1]);

        c = dc_session_channel_by_id(current_session, id);
        if (c == NULL) {
            LOG(n, L"join: no channel found with that snowflake: %s", id);
            goto cleanup;
        }
    }

    if (dc_channel_messages(c) == 0) {
        bool ret = false;

        ret = dc_api_get_messages(dc_session_api(current_session),
                                  dc_session_me(current_session),
                                  c
            );
        if (!ret) {
            LOG(n, L"join: failed to fetch messages for channel %s",
                dc_channel_name(c)
                );
            goto cleanup;
        }
    }

    /* this adds a channel, or switches to the channel if a view already exists
     */
    ncdc_mainwindow_switch_or_add(n, c);

    ret = true;

cleanup:

    free(guild);
    free(channel);
    free(id);

    return ret;
}
