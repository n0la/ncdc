CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

FIND_PACKAGE(PkgConfig)
FIND_PACKAGE(Threads)

PKG_CHECK_MODULES(JANSSON REQUIRED jansson)
PKG_CHECK_MODULES(CURL REQUIRED libcurl)
PKG_CHECK_MODULES(EVENT REQUIRED libevent libevent_pthreads)
PKG_CHECK_MODULES(GLIB2 REQUIRED glib-2.0)

SET(TARGET "ncdc")

SET(SOURCES
  "include/ncdc/account.h"
  "include/ncdc/api.h"
  "include/ncdc/apisync.h"
  "include/ncdc/ncdc.h"
  "include/ncdc/refable.h"
  "src/account.c"
  "src/api.c"
  "src/apisync.c"
  "src/ncdc.c"
  "src/refable.c"
  )

ADD_DEFINITIONS("-Wall -Werror -std=c11 -D_GNU_SOURCE")

INCLUDE_DIRECTORIES("include"
  ${JANSSON_INCLUDE_DIRS}
  ${CURL_INCLUDE_DIRS}
  ${EVENT_INCLUDE_DIRS}
  ${GLIB2_INCLUDE_DIRS}
  )
LINK_DIRECTORIES(${JANSSON_LIBRARY_DIRS}
  ${CURL_LIBRARY_DIRS}
  ${EVENT_LIBRARY_DIRS}
  ${GLIB2_LIBRARY_DIRS}
  )

ADD_EXECUTABLE(${TARGET} ${SOURCES})
TARGET_LINK_LIBRARIES(${TARGET}
  ${JANSSON_LIBRARIES}
  ${CURL_LIBRARIES}
  ${EVENT_LIBRARIES}
  ${GLIB2_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  )
