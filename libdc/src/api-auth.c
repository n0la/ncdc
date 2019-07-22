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

#include <dc/api.h>
#include "internal.h"

bool dc_api_logout(dc_api_t api, dc_account_t account)
{
    json_t *reply = NULL;
    json_t *data = NULL;

    data = json_object();
    return_if_true(data == NULL, false);

    json_object_set_new(data, "provider", json_null());
    json_object_set_new(data, "voip_provider", json_null());

    reply = dc_api_call_sync(api, "POST", dc_account_token(account),
                             "auth/logout", data
        );
    json_decref(data);

    if (reply != NULL) {
        /* TODO: parse error
         */
        json_decref(reply);
        return false;
    }

    dc_account_set_token(account, NULL);
    return true;
}

bool dc_api_login(dc_api_t api, dc_account_t account)
{
    if (!dc_api_authenticate(api, account)) {
        goto cleanup;
    }

    if (!dc_api_get_userinfo(api, account, account)) {
        goto cleanup;
    }

    if (!dc_api_get_friends(api, account)) {
        goto cleanup;
    }

    return true;

cleanup:

    dc_account_set_token(account, NULL);

    return false;
}

bool dc_api_authenticate(dc_api_t api, dc_account_t account)
{
    json_t *j = json_object(), *reply = NULL, *token = NULL;
    bool ret = false;

    json_object_set_new(j, "email",
                        json_string(dc_account_email(account))
        );
    json_object_set_new(j, "password",
                        json_string(dc_account_password(account))
        );

    reply = dc_api_call_sync(api, "POST", NULL, "auth/login", j);
    goto_if_true(reply == NULL, cleanup);

    if (dc_api_error(j, NULL, NULL)) {
        return false;
    }

    token = json_object_get(reply, "token");
    if (token == NULL || !json_is_string(token)) {
        goto cleanup;
    }

    dc_account_set_token(account, json_string_value(token));
    ret = true;

cleanup:

    if (j != NULL) {
        json_decref(j);
        j = NULL;
    }

    if (reply != NULL) {
        json_decref(reply);
        reply = NULL;
    }

    return ret;
}
