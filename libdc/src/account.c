#include <dc/account.h>
#include <dc/refable.h>

#include "internal.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct dc_account_
{
    dc_refable_t ref; /* do not move anything above ref */

    char *email;
    char *password;

    /* internal ID
     */
    char *id;
    /* username
     */
    char *username;
    /* discriminator
     */
    char *discriminator;
    /* full username username#discriminator
     */
    char *full;
    /* online/offline status
     */
    char *status;

    /* authentication token
     */
    char *token;

    /* friends we have
     */
    GPtrArray *friends;
    /* our own friend state
     */
    int friend_state;
};

static void dc_account_free(dc_account_t ptr)
{
    return_if_true(ptr == NULL,);

    free(ptr->email);
    free(ptr->password);
    free(ptr->id);
    free(ptr->username);
    free(ptr->discriminator);
    free(ptr->full);
    free(ptr->token);
    free(ptr->status);

    if (ptr->friends != NULL) {
        g_ptr_array_unref(ptr->friends);
        ptr->friends = NULL;
    }

    free(ptr);
}

dc_account_t dc_account_new(void)
{
    dc_account_t ptr = calloc(1, sizeof(struct dc_account_));

    ptr->ref.cleanup = (dc_cleanup_t)dc_account_free;

    ptr->friends = g_ptr_array_new_with_free_func(
        (GDestroyNotify)dc_unref
        );

    return dc_ref(ptr);
}

dc_account_t dc_account_new2(char const *email, char const *pass)
{
    dc_account_t ptr = dc_account_new();

    if (ptr != NULL) {
        dc_account_set_email(ptr, email);
        dc_account_set_password(ptr, pass);
    }

    return ptr;
}

dc_account_t dc_account_from_fullname(char const *fullid)
{
    return_if_true(fullid == NULL, NULL);

    char *name = strdup(fullid), *discriminator = NULL;
    dc_account_t acc = NULL;

    return_if_true(name == NULL, NULL);

    discriminator = strchr(name, '#');
    if (discriminator == NULL || *discriminator == '\0') {
        free(name);
        return NULL;
    }

    *discriminator = '\0';
    ++discriminator;

    acc = dc_account_new();
    if (acc == NULL) {
        free(name);
        return NULL;
    }

    dc_account_set_username(acc, name);
    dc_account_set_discriminator(acc, discriminator);

    free(name);

    return acc;
}

bool dc_account_load(dc_account_t user, json_t *j)
{
    json_t *val = NULL;

    return_if_true(!json_is_object(j), false);

    val = json_object_get(j, "id");
    return_if_true(val == NULL || !json_is_string(val), false);
    dc_account_set_id(user, json_string_value(val));

    val = json_object_get(j, "username");
    return_if_true(val == NULL || !json_is_string(val), false);
    dc_account_set_username(user, json_string_value(val));

    val = json_object_get(j, "discriminator");
    return_if_true(val == NULL || !json_is_string(val), false);
    dc_account_set_discriminator(user, json_string_value(val));

    return true;
}

dc_account_t dc_account_from_relationship(json_t *j)
{
    dc_account_t user = dc_account_new();
    json_t *val = NULL;

    val = json_object_get(j, "user");
    goto_if_true(val == NULL || !json_is_object(val), error);

    if (!dc_account_load(user, val)) {
        goto error;
    }

    val = json_object_get(j, "status");
    if (val != NULL && json_is_string(val)) {
        dc_account_set_status(user, json_string_value(val));
    }

    return user;

error:

    dc_unref(user);
    return NULL;
}

dc_account_t dc_account_from_json(json_t *j)
{
    dc_account_t user = dc_account_new();

    if (!dc_account_load(user, j)) {
        dc_unref(user);
        return NULL;
    }

    return user;
}

json_t *dc_account_to_json(dc_account_t a)
{
    json_t *j = NULL;

    return_if_true(a == NULL, NULL);
    return_if_true(dc_account_username(a) == NULL ||
                   dc_account_discriminator(a) == NULL,
                   NULL
        );

    j = json_object();
    return_if_true(j == NULL, NULL);

    if (a->id != NULL) {
        json_object_set_new(j, "id", json_string(a->id));
    }

    json_object_set_new(j, "username", json_string(a->username));
    json_object_set_new(j, "discriminator", json_string(a->discriminator));

    return j;
}

void dc_account_set_email(dc_account_t a, char const *email)
{
    return_if_true(a == NULL,);
    return_if_true(email == NULL,);

    free(a->email);
    a->email = strdup(email);
}

char const *dc_account_email(dc_account_t a)
{
    return_if_true(a == NULL, NULL);
    return a->email;
}

void dc_account_set_password(dc_account_t a, char const *password)
{
    return_if_true(a == NULL,);
    return_if_true(password == NULL,);

    free(a->password);
    a->password = strdup(password);
}

char const *dc_account_password(dc_account_t a)
{
    return_if_true(a == NULL, NULL);
    return a->password;
}

void dc_account_set_token(dc_account_t a, char const *token)
{
    return_if_true(a == NULL,);

    free(a->token);
    a->token = NULL;

    if (token != NULL) {
        a->token = strdup(token);
    }
}

char const *dc_account_token(dc_account_t a)
{
    return_if_true(a == NULL, NULL);
    return a->token;
}

bool dc_account_has_token(dc_account_t a)
{
    return_if_true(a == NULL, false);
    return_if_true(a->token == NULL, false);
    return true;
}

void dc_account_set_id(dc_account_t a, char const *id)
{
    return_if_true(a == NULL,);

    free(a->id);
    a->id = strdup(id);
}

char const *dc_account_id(dc_account_t a)
{
    return_if_true(a == NULL,NULL);
    return a->id;
}

void dc_account_update_full(dc_account_t a)
{
    free(a->full);
    a->full = NULL;

    asprintf(&a->full, "%s#%s",
             (a->username != NULL ? a->username : ""),
             (a->discriminator != NULL ? a->discriminator : "")
        );
}

void dc_account_set_username(dc_account_t a, char const *id)
{
    return_if_true(a == NULL,);

    free(a->username);
    a->username = strdup(id);
    dc_account_update_full(a);
}

char const *dc_account_username(dc_account_t a)
{
    return_if_true(a == NULL, NULL);
    return a->username;
}

void dc_account_set_discriminator(dc_account_t a, char const *id)
{
    return_if_true(a == NULL,);

    free(a->discriminator);
    a->discriminator = strdup(id);
    dc_account_update_full(a);
}

char const *dc_account_discriminator(dc_account_t a)
{
    return_if_true(a == NULL, NULL);
    return a->discriminator;
}

char const *dc_account_fullname(dc_account_t a)
{
    return_if_true(a == NULL, NULL);
    return a->full;
}

char const *dc_account_status(dc_account_t a)
{
    return_if_true(a == NULL, NULL);
    return a->status;
}

void dc_account_set_status(dc_account_t a, char const *s)
{
    return_if_true(a == NULL || s == NULL,);
    free(a->status);
    a->status = strdup(s);
}

bool dc_account_equal(dc_account_t a, dc_account_t b)
{
    return_if_true(a == NULL && b == NULL, true);
    return_if_true(a == NULL || b == NULL, false);
    return (strcmp(a->full, b->full) == 0);
}

void dc_account_set_friends(dc_account_t a, dc_account_t *friends, size_t len)
{
    size_t i = 0;
    return_if_true(a == NULL || a->friends == NULL,);

    g_ptr_array_remove_range(a->friends, 0, a->friends->len);
    for (i = 0; i < len; i++) {
        g_ptr_array_add(a->friends, dc_ref(friends[i]));
    }
}

dc_account_t dc_account_find_friend(dc_account_t a, char const *fullname)
{
    size_t i = 0;
    return_if_true(a == NULL || fullname == NULL, NULL);

    for (i = 0; i < a->friends->len; i++) {
        dc_account_t f = g_ptr_array_index(a->friends, i);
        if (strcmp(f->full, fullname) == 0) {
            return f;
        }
    }

    return NULL;
}

void dc_account_add_friend(dc_account_t a, dc_account_t friend)
{
    return_if_true(a == NULL || friend == NULL,);
    g_ptr_array_add(a->friends, dc_ref(friend));
}

dc_account_t dc_account_nth_friend(dc_account_t a, size_t i)
{
    return_if_true(a == NULL || a->friends == NULL, NULL);
    return (dc_account_t)g_ptr_array_index(a->friends, i);
}

size_t dc_account_friends_size(dc_account_t a)
{
    return_if_true(a == NULL || a->friends == NULL, 0);
    return a->friends->len;
}

int dc_account_friend_state(dc_account_t a)
{
    return_if_true(a == NULL, 0);
    return a->friend_state;
}

void dc_account_set_friend_state(dc_account_t a, int state)
{
    return_if_true(a == NULL,);
    a->friend_state = state;
}
