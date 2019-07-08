#ifndef DC_INTERNAL_H
#define DC_INTERNAL_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include <pthread.h>

#include <curl/curl.h>

#include <jansson.h>

#include <glib.h>

#include <event.h>
#include <event2/thread.h>

#include <dc/util.h>
#include <dc/refable.h>
#include <dc/account.h>

//#define DEBUG

#define return_if_true(v,r) do { if (v) return r; } while(0)
#define goto_if_true(v,l) do { if (v) goto l; } while(0)

#define TOKEN(l) (dc_account_token(l))

#endif
