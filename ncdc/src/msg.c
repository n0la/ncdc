#include <ncdc/cmds.h>
#include <ncdc/ncdc.h>
#include <ncdc/textview.h>

bool ncdc_cmd_msg(ncdc_mainwindow_t n, size_t ac,
                  wchar_t **av, wchar_t const *fullmsg)
{
    return_if_true(ac <= 1, false);

    char * target = NULL;
    wchar_t *full_message = NULL;
    char * message = NULL;
    dc_message_t m = NULL;
    bool ret = false;
    dc_channel_t c = NULL;
    ncdc_textview_t v = NULL;
    dc_account_t current_account = NULL;
    size_t i = 0;

    if (!is_logged_in()) {
        LOG(n, L"msg: not logged in");
        return false;
    }

    target = w_convert(av[1]);
    goto_if_true(target == NULL, cleanup);

    current_account = dc_session_me(current_session);

    /* find out if the target is a friend we can contact
     */
    dc_account_t f = dc_session_account_fullname(current_session, target);
    if (f == NULL) {
        LOG(n, L"msg: no such account found: \"%s\"", target);
        goto cleanup;
    }

    c = dc_session_make_channel(current_session, &f, 1);
    if (c == NULL) {
        LOG(n, L"msg: failed to create channel for these recipients");
        goto cleanup;
    }

    /* see if we have a channel already, that services that user
     * if so we set v to something non-NIL and it should be good
     */
    for (i = 0; i < ncdc_mainwindow_views(n)->len; i++) {
        ncdc_textview_t view = g_ptr_array_index(ncdc_mainwindow_views(n), i);
        dc_channel_t chan = ncdc_textview_channel(view);

        if (dc_channel_compare(chan, c)) {
            ncdc_mainwindow_switchview(n, i);
            v = view;
            break;
        }
    }

    if (v == NULL) {
        v = ncdc_textview_new();
        goto_if_true(v == NULL, cleanup);

        ncdc_textview_set_account(v, current_account);
        ncdc_textview_set_channel(v, c);

        g_ptr_array_add(ncdc_mainwindow_views(n), dc_ref(v));
        ncdc_mainwindow_switchview(n, ncdc_mainwindow_views(n)->len-1);
    }

    if (ac > 2) {
        /* also post the rest of the content as a message to the channel
         */
        full_message = wcsstr(fullmsg, av[2]);
        goto_if_true(full_message == NULL, cleanup);

        message = w_convert(full_message);
        goto_if_true(message == NULL, cleanup);

        m = dc_message_new_content(message, -1);
        goto_if_true(m == NULL, cleanup);

        ret = dc_api_post_message(
            dc_session_api(current_session),
            dc_session_me(current_session),
            c, m
            );
    }

    ret = true;

cleanup:

    dc_unref(c);
    dc_unref(v);
    dc_unref(m);

    free(target);
    free(message);

    return ret;
}
