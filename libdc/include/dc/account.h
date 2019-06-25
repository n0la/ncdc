#ifndef DC_ACCOUNT_H
#define DC_ACCOUNT_H

#include <stdint.h>
#include <stdbool.h>

struct dc_account_;
typedef struct dc_account_ *dc_account_t;

dc_account_t dc_account_new(void);
dc_account_t dc_account_new2(char const *email, char const *pass);

void dc_account_set_email(dc_account_t a, char const *email);
char const *dc_account_email(dc_account_t a);

void dc_account_set_password(dc_account_t a, char const *password);
char const *dc_account_password(dc_account_t a);

void dc_account_set_id(dc_account_t a, char const *id);
char const *dc_account_id(dc_account_t a);

void dc_account_set_token(dc_account_t a, char const *token);
char const *dc_account_token(dc_account_t a);
bool dc_account_has_token(dc_account_t a);

#endif
