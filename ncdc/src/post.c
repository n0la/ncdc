#include <ncdc/cmds.h>
#include <ncdc/ncdc.h>

bool ncdc_cmd_post(ncdc_mainwindow_t n, size_t ac, wchar_t **av,
                   wchar_t const *f)
{
    char *str = NULL;
    bool ret = false;
    dc_message_t m = NULL;
    dc_channel_t chan = NULL;
    size_t i = 0;

    if (!is_logged_in()) {
        return false;
    }

    chan = ncdc_mainwindow_current_channel(n);
    goto_if_true(chan == NULL, cleanup);

    /* the API only uses multi-byte strings, so convert from wchar_t
     */
    str = w_convert(f);
    goto_if_true(str == NULL, cleanup);

    /* trim string at least on the left
     */
    for (i = 0; isspace(str[i]) && str[i] != '\0'; i++)
        ;
    goto_if_true (str[i] == '\0', cleanup);
    memmove(str, str+i, strlen(str)+1-i);

    m = dc_message_new_content(str, -1);
    goto_if_true(m == NULL, cleanup);

    ret = dc_api_post_message(
        dc_session_api(current_session),
        dc_session_me(current_session),
        chan, m
        );
    goto_if_true(ret == false, cleanup);

    ret = true;

cleanup:

    dc_unref(m);
    free(str);

    return ret;
}
