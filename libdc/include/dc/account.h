#ifndef DC_ACCOUNT_H
#define DC_ACCOUNT_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <jansson.h>

struct dc_account_;
typedef struct dc_account_ *dc_account_t;

typedef enum {
    FRIEND_STATE_NONE = 0,
    /* accountt is a mutual friend
     */
    FRIEND_STATE_FRIEND = 1,
    /* pending friend request, the other side hasn't accepted yet
     */
    FRIEND_STATE_PENDING = 3,
} dc_account_friend_states;

/**
 * Represents a given account within the discord system. To start your work,
 * you will have to create an account object, give it email and password, and
 * call the dc_api_authenticate() with it. This gives you a login token that
 * you can use to call other API methods (such as dc_api_get_friends()) for
 * that account.
 *
 * Accounts have a few important attributes that we store:
 *   * ID (or snowflake), a 64 bit ID of the user, that we store as a string.
 *   * username, a string that represents the accounts user name
 *   * discriminator, a number that differentiates users with the same name
 *   * email, for login accounts only
 *   * password, for login accounts only
 *   * friend_state, if the account is someone login account's friend, we store
 *     the relationship in this flag. See the dc_account_friend_state enum for
 *     details.
 * And one compound attribute:
 *   * fullname, a combination of username and discriminator separated by the
 *     pound sign, e.g. nola#2457
 */

dc_account_t dc_account_new(void);
dc_account_t dc_account_new2(char const *email, char const *pass);
dc_account_t dc_account_from_fullname(char const *fullid);

dc_account_t dc_account_from_json(json_t *j);
dc_account_t dc_account_from_relationship(json_t *j);
json_t *dc_account_to_json(dc_account_t a);
bool dc_account_load(dc_account_t a, json_t *j);

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

char const *dc_account_fullname(dc_account_t a);

char const *dc_account_status(dc_account_t a);
void dc_account_set_status(dc_account_t a, char const *s);

void dc_account_set_token(dc_account_t a, char const *token);
char const *dc_account_token(dc_account_t a);
bool dc_account_has_token(dc_account_t a);

/* compare
 */
bool dc_account_equal(dc_account_t a, dc_account_t b);

/* relationships
 */
void dc_account_set_friends(dc_account_t a, dc_account_t *ptr, size_t len);
void dc_account_add_friend(dc_account_t a, dc_account_t friend);
dc_account_t dc_account_nth_friend(dc_account_t a, size_t i);
size_t dc_account_friends_size(dc_account_t a);
dc_account_t dc_account_find_friend(dc_account_t a, char const *fullname);

int dc_account_friend_state(dc_account_t a);
void dc_account_set_friend_state(dc_account_t a, int state);

#endif
