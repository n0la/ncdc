#include <ncdc/cmds.h>
#include <ncdc/ncdc.h>

bool
ncdc_cmd_join(ncdc_mainwindow_t n, size_t ac, wchar_t **av, wchar_t const *f)
{
    char *guild = NULL;
    char *channel = NULL;
    bool ret = true;
    dc_guild_t g = NULL;
    dc_channel_t c = NULL;

    if (ac <= 2) {
        LOG(n, L"join: not enough arguments given");
        return false;
    }

    if (!is_logged_in()) {
        return false;
    }

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

    if (dc_channel_messages(c) == 0) {
        bool ret = false;

        ret = dc_api_get_messages(dc_session_api(current_session),
                                  dc_session_me(current_session),
                                  c
            );
        if (!ret) {
            LOG(n, L"join: failed to fetch messages for channel %s", channel);
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

    return ret;
}
