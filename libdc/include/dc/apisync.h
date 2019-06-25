#ifndef DC_API_ASYNC_H
#define DC_API_ASYNC_H

#include <curl/curl.h>

#include <stdio.h>
#include <stdbool.h>

struct dc_api_sync_;
typedef struct dc_api_sync_ *dc_api_sync_t;

dc_api_sync_t dc_api_sync_new(CURLM *curl, CURL *easy);

FILE *dc_api_sync_stream(dc_api_sync_t sync);
char const *dc_api_sync_data(dc_api_sync_t sync);
size_t dc_api_sync_datalen(dc_api_sync_t sync);
int dc_api_sync_code(dc_api_sync_t sync);
struct curl_slist *dc_api_sync_list(dc_api_sync_t sync);

bool dc_api_sync_wait(dc_api_sync_t sync);
void dc_api_sync_finish(dc_api_sync_t sync, int code);

#endif
