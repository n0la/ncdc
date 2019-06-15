#include <ncdc/account.h>
#include <ncdc/refable.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct ncdc_account_
{
    ncdc_refable_t ref; /* do not move anything above ref */

    char *email;
    char *password;

    /* internal ID
     */
    char *id;

    /* authentication token
     */
    char *token;
};

static void ncdc_account_free(ncdc_account_t ptr)
{
    return_if_true(ptr == NULL,);

    free(ptr->email);
    free(ptr->password);

    free(ptr);
}

ncdc_account_t ncdc_account_new(void)
{
    ncdc_account_t ptr = calloc(1, sizeof(struct ncdc_account_));

    ptr->ref.cleanup = (cleanup_t)ncdc_account_free;

    return ncdc_ref(ptr);
}

ncdc_account_t ncdc_account_new2(char const *email, char const *pass)
{
    ncdc_account_t ptr = ncdc_account_new();

    if (ptr != NULL) {
        ncdc_account_set_email(ptr, email);
        ncdc_account_set_password(ptr, pass);
    }

    return ptr;
}

void ncdc_account_set_email(ncdc_account_t a, char const *email)
{
    return_if_true(a == NULL,);
    return_if_true(email == NULL,);

    free(a->email);
    a->email = strdup(email);
}

char const *ncdc_account_email(ncdc_account_t a)
{
    return_if_true(a == NULL, NULL);
    return a->email;
}

void ncdc_account_set_password(ncdc_account_t a, char const *password)
{
    return_if_true(a == NULL,);
    return_if_true(password == NULL,);

    free(a->password);
    a->password = strdup(password);
}

char const *ncdc_account_password(ncdc_account_t a)
{
    return_if_true(a == NULL, NULL);
    return a->password;
}

void ncdc_account_set_token(ncdc_account_t a, char const *token)
{
    return_if_true(a == NULL,);

    free(a->token);
    a->token = NULL;

    if (token != NULL) {
        a->token = strdup(token);
    }
}

char const *ncdc_account_token(ncdc_account_t a)
{
    return_if_true(a == NULL, NULL);
    return a->token;
}

bool ncdc_account_has_token(ncdc_account_t a)
{
    return_if_true(a == NULL, false);
    return_if_true(a->token == NULL, false);
    return true;
}

void ncdc_account_set_id(ncdc_account_t a, char const *id)
{
    return_if_true(a == NULL,);
    free(a->id);
    a->id = strdup(id);
}

char const *ncdc_account_id(ncdc_account_t a)
{
    return_if_true(a == NULL,NULL);
    return a->id;
}
