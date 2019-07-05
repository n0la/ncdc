CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PKG_CHECK_MODULES(NCURSES REQUIRED ncursesw)
PKG_CHECK_MODULES(PANEL REQUIRED panel)
PKG_CHECK_MODULES(CONFUSE REQUIRED libconfuse)

SET(TARGET "ncdc")

SET(SOURCES
  "include/ncdc/cmds.h"
  "include/ncdc/config.h"
  "include/ncdc/input.h"
  "include/ncdc/mainwindow.h"
  "include/ncdc/ncdc.h"
  "include/ncdc/textview.h"
  "src/cmds.c"
  "src/config.c"
  "src/emacs.c"
  "src/friends.c"
  "src/input.c"
  "src/login.c"
  "src/mainwindow.c"
  "src/msg.c"
  "src/ncdc.c"
  "src/textview.c"
  "src/util.c"
  )

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${DC_INCLUDE_DIRS}
  ${JANSSON_INCLUDE_DIRS}
  ${CURL_INCLUDE_DIRS}
  ${EVENT_INCLUDE_DIRS}
  ${GLIB2_INCLUDE_DIRS}
  ${NCURSES_INCLUDE_DIRS}
  ${PANEL_INCLUDE_DIRS}
  ${CONFUSE_INCLUDE_DIRS}
  )

ADD_EXECUTABLE(${TARGET} ${SOURCES})
TARGET_LINK_LIBRARIES(${TARGET}
  ${DC_LIBRARIES}
  ${GLIB2_LIBRARIES}
  ${NCURSES_LIBRARIES}
  ${PANEL_LIBRARIES}
  ${CONFUSE_LIBRARIES}
  )
