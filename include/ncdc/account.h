#ifndef NCDC_ACCOUNT_H
#define NCDC_ACCOUNT_H

#include <ncdc/ncdc.h>

struct ncdc_account_;
typedef struct ncdc_account_ *ncdc_account_t;

ncdc_account_t ncdc_account_new(void);
ncdc_account_t ncdc_account_new2(char const *email, char const *pass);

void ncdc_account_set_email(ncdc_account_t a, char const *email);
char const *ncdc_account_email(ncdc_account_t a);

void ncdc_account_set_password(ncdc_account_t a, char const *password);
char const *ncdc_account_password(ncdc_account_t a);

void ncdc_account_set_id(ncdc_account_t a, char const *id);
char const *ncdc_account_id(ncdc_account_t a);

void ncdc_account_set_token(ncdc_account_t a, char const *token);
char const *ncdc_account_token(ncdc_account_t a);
bool ncdc_account_has_token(ncdc_account_t a);

#endif
