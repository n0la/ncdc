#ifndef NCDC_API_ASYNC_H
#define NCDC_API_ASYNC_H

#include <ncdc/ncdc.h>

struct ncdc_api_sync_;
typedef struct ncdc_api_sync_ *ncdc_api_sync_t;

ncdc_api_sync_t ncdc_api_sync_new(CURLM *curl, CURL *easy);

FILE *ncdc_api_sync_stream(ncdc_api_sync_t sync);
char const *ncdc_api_sync_data(ncdc_api_sync_t sync);
size_t ncdc_api_sync_datalen(ncdc_api_sync_t sync);
int ncdc_api_sync_code(ncdc_api_sync_t sync);
struct curl_slist *ncdc_api_sync_list(ncdc_api_sync_t sync);

bool ncdc_api_sync_wait(ncdc_api_sync_t sync);
void ncdc_api_sync_finish(ncdc_api_sync_t sync, int code);

#endif
