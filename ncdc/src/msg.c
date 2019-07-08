#include <ncdc/cmds.h>
#include <ncdc/ncdc.h>
#include <ncdc/textview.h>

bool ncdc_cmd_msg(ncdc_mainwindow_t n, size_t ac, wchar_t **av)
{
    return_if_true(ac <= 1, false);

    char * target = NULL;
    wchar_t *full_message = NULL;
    char * message = NULL;
    bool ret = false;
    dc_channel_t c = NULL;
    ncdc_textview_t v = NULL;
    size_t i = 0;

    if (current_account == NULL || !dc_account_has_token(current_account)) {
        LOG(n, L"msg: not logged in");
        return false;
    }

    target = w_convert(av[1]);
    goto_if_true(target == NULL, cleanup);

    /* find out if the target is a friend we can contact
     */
    dc_account_t f = dc_account_findfriend(current_account, target);
    if (f == NULL) {
        LOG(n, L"msg: no such friend found: \"%s\"", target);
        goto cleanup;
    }

    /* see if we have a channel already, that services that user
     */
    for (i = 0; i < ncdc_mainwindow_views(n)->len; i++) {
        v = g_ptr_array_index(ncdc_mainwindow_views(n), i);
        dc_channel_t chan = ncdc_textview_channel(v);

        if (chan != NULL &&
            dc_channel_type(chan) == CHANNEL_TYPE_DM_TEXT &&
            dc_account_equal(dc_channel_nthrecipient(chan, 1), f)) {
            c = dc_ref(chan);
            ncdc_mainwindow_switchview(n, i);
            break;
        }
    }

    if (c == NULL) {
        /* no? create a new window and switch to it
         */
        if (!dc_api_create_channel(api, current_account, &f, 1, &c)) {
            LOG(n, L"msg: failed to create channel");
            goto cleanup;
        }

        if (!dc_api_get_messages(api, current_account, c)) {
            LOG(n, L"msg: failed to fetch messages in channel");
            goto cleanup;
        }

        v = ncdc_textview_new();
        goto_if_true(v == NULL, cleanup);

        ncdc_textview_set_account(v, current_account);
        ncdc_textview_set_channel(v, c);

        g_ptr_array_add(ncdc_mainwindow_views(n), dc_ref(v));
        ncdc_mainwindow_switchview(n, ncdc_mainwindow_views(n)->len-1);
    }

    ret = true;

cleanup:

    dc_unref(c);
    dc_unref(v);

    free(target);
    free(full_message);
    free(message);

    return ret;
}
