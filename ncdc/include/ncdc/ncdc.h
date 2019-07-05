#ifndef NCDC_H
#define NCDC_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <wctype.h>

#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include <pthread.h>

#include <curl/curl.h>

#include <jansson.h>

#include <glib.h>

#include <event.h>
#include <event2/thread.h>

#include <curses.h>
#include <panel.h>

#include <locale.h>

#include <dc/refable.h>
#include <dc/api.h>
#include <dc/loop.h>
#include <dc/account.h>

#define return_if_true(v,r) do { if (v) return r; } while(0)
#define goto_if_true(v,l) do { if (v) goto l; } while(0)

struct ncdc_account_ {
    dc_account_t account;
    GPtrArray *friends;
    GPtrArray *guilds;
};

typedef struct ncdc_account_ *ncdc_account_t;

extern GHashTable *accounts;
extern dc_account_t current_account;

extern dc_api_t api;

extern char *ncdc_private_dir;
extern void *config;
extern void *mainwindow;

void exit_main(void);

int strwidth(char const *string);
char *read_char(FILE *stream);

int aswprintf(wchar_t **buffer, wchar_t const *fmt, ...);
char *w_convert(wchar_t const *w);
wchar_t* wcsndup(const wchar_t* string, size_t maxlen);
size_t w_strlenv(wchar_t **s);
void w_strfreev(wchar_t **s);
wchar_t *w_joinv(wchar_t const **v, size_t len);
wchar_t **w_tokenise(wchar_t const *w);
wchar_t *w_next_tok(wchar_t const *w);
wchar_t const *w_next_word(wchar_t const *w, ssize_t len);

#endif
