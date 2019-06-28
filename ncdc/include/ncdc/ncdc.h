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

#define return_if_true(v,r) do { if (v) return r; } while(0)
#define goto_if_true(v,l) do { if (v) goto l; } while(0)

extern char *ncdc_private_dir;

int strwidth(char const *string);
char *read_char(FILE *stream);

wchar_t const *w_next_word(wchar_t const *w, ssize_t len);

#endif
