CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

SET(TARGET "dc")

SET(DC_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include" CACHE STRING
  "Discord API include directories"
  )
SET(DC_LIBRARIES "${TARGET}" CACHE STRING "Discord API libraries")

SET(SOURCES
  "include/dc/account.h"
  "include/dc/api.h"
  "include/dc/apisync.h"
  "include/dc/channel.h"
  "include/dc/guild.h"
  "include/dc/loop.h"
  "include/dc/message.h"
  "include/dc/refable.h"
  "include/dc/util.h"
  "src/account.c"
  "src/api.c"
  "src/api-auth.c"
  "src/api-channel.c"
  "src/api-friends.c"
  "src/apisync.c"
  "src/channel.c"
  "src/guild.c"
  "src/loop.c"
  "src/message.c"
  "src/refable.c"
  "src/util.c"
  )

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

ADD_LIBRARY(${TARGET} SHARED ${SOURCES})
TARGET_LINK_LIBRARIES(${TARGET}
  ${JANSSON_LIBRARIES}
  ${CURL_LIBRARIES}
  ${EVENT_LIBRARIES}
  ${GLIB2_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  )
