#include <dc/api.h>

#include "internal.h"

dc_account_t dc_api_account_from_json(json_t *j)
{
    dc_account_t user = dc_account_new();
    json_t *val = NULL;

    goto_if_true(!json_is_object(j), error);

    val = json_object_get(j, "username");
    goto_if_true(val == NULL || !json_is_string(val), error);
    dc_account_set_username(user, json_string_value(val));

    val = json_object_get(j, "discriminator");
    goto_if_true(val == NULL || !json_is_string(val), error);
    dc_account_set_discriminator(user, json_string_value(val));

    val = json_object_get(j, "id");
    goto_if_true(val == NULL || !json_is_string(val), error);
    dc_account_set_id(user, json_string_value(val));

    return user;

error:

    dc_unref(user);
    return NULL;
}

json_t *dc_api_account_to_json(dc_account_t a)
{
    json_t *j = NULL;

    return_if_true(a == NULL, NULL);
    return_if_true(dc_account_username(a) == NULL ||
                   dc_account_discriminator(a) == NULL,
                   NULL
        );

    j = json_object();
    return_if_true(j == NULL, NULL);

    json_object_set_new(j, "username",
                        json_string(dc_account_username(a))
        );
    json_object_set_new(j, "discriminator",
                        json_string(dc_account_discriminator(a))
        );

    return j;
}
