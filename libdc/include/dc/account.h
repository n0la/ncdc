#ifndef DC_ACCOUNT_H
#define DC_ACCOUNT_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

struct dc_account_;
typedef struct dc_account_ *dc_account_t;

typedef enum {
    FRIEND_STATE_NONE = 0,
    /* accountt is a mutual friend
     */
    FRIEND_STATE_FRIEND = 1,
    /* pending account, the other side hasn't accepted yet
     */
    FRIEND_STATE_PENDING = 4,
} dc_account_friend_states;

dc_account_t dc_account_new(void);
dc_account_t dc_account_new2(char const *email, char const *pass);
dc_account_t dc_account_from_fullid(char const *fullid);

void dc_account_set_email(dc_account_t a, char const *email);
char const *dc_account_email(dc_account_t a);

void dc_account_set_password(dc_account_t a, char const *password);
char const *dc_account_password(dc_account_t a);

void dc_account_set_id(dc_account_t a, char const *id);
char const *dc_account_id(dc_account_t a);

void dc_account_set_username(dc_account_t a, char const *id);
char const *dc_account_username(dc_account_t a);

void dc_account_set_discriminator(dc_account_t a, char const *id);
char const *dc_account_discriminator(dc_account_t a);

char const *dc_account_full_username(dc_account_t a);

void dc_account_set_token(dc_account_t a, char const *token);
char const *dc_account_token(dc_account_t a);
bool dc_account_has_token(dc_account_t a);

/* relationships
 */
void dc_account_set_friends(dc_account_t a, dc_account_t *ptr, size_t len);
dc_account_t dc_account_nthfriend(dc_account_t a, size_t i);
size_t dc_account_friends_size(dc_account_t a);
int dc_account_friend_state(dc_account_t a);
void dc_account_set_friend_state(dc_account_t a, int state);

#endif
