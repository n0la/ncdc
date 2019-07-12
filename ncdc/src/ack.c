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
