#ifndef NCDC_API_H
#define NCDC_API_H

#include <ncdc/ncdc.h>

#include <ncdc/apisync.h>
#include <ncdc/account.h>

struct ncdc_api_;
typedef struct ncdc_api_ *ncdc_api_t;

ncdc_api_t ncdc_api_new(void);

void ncdc_api_set_curl_multi(ncdc_api_t api, CURLM *curl);
void ncdc_api_set_event_base(ncdc_api_t api, struct event_base *base);

/* call this function in case the MULTI has told us that some
 * transfer has finished.
 */
void ncdc_api_signal(ncdc_api_t api, CURL *easy, int code);

/* internal curl stuff
 */
ncdc_api_sync_t ncdc_api_call(ncdc_api_t api, char const *token,
                              char const *method, json_t *j);
json_t *ncdc_api_call_sync(ncdc_api_t api, char const *token,
                           char const *method, json_t *j);

bool ncdc_api_authenticate(ncdc_api_t api, ncdc_account_t account);
bool ncdc_api_userinfo(ncdc_api_t api, ncdc_account_t logion,
                       ncdc_account_t user);


#endif
