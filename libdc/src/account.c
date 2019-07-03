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

    /* authentication token
     */
    char *token;

    /* friends we have
     */
    GPtrArray *friends;
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

char const *dc_account_full_username(dc_account_t a)
{
    return_if_true(a == NULL, NULL);
    return a->full;
}

void dc_account_set_friends(dc_account_t a, dc_account_t *friends, size_t len)
{
    size_t i = 0;
    return_if_true(a == NULL || a->friends == NULL,);

    g_ptr_array_remove_range(a->friends, 0, a->friends->len);
    for (i = 0; i < len; i++) {
        g_ptr_array_add(a->friends, friends[i]);
    }
}

dc_account_t dc_account_nthfriend(dc_account_t a, size_t i)
{
    return_if_true(a == NULL || a->friends == NULL, NULL);
    return (dc_account_t)g_ptr_array_index(a->friends, i);
}

size_t dc_account_friends_size(dc_account_t a)
{
    return_if_true(a == NULL || a->friends == NULL, 0);
    return a->friends->len;
}
