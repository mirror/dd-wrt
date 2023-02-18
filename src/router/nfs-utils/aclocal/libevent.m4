dnl Checks for libevent
AC_DEFUN([AC_LIBEVENT], [

  dnl Check for libevent, but do not add -levent_core to LIBS
  AC_CHECK_LIB([event_core], [event_base_dispatch], [LIBEVENT=-levent_core],
               [AC_MSG_ERROR([libevent not found.])])
  AC_SUBST(LIBEVENT)

  AC_CHECK_HEADERS([event2/event.h], ,
                   [AC_MSG_ERROR([libevent headers not found.])])

])dnl
