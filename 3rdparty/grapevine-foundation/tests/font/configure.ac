AC_INIT([Font Test], 1.0)
AC_DEFINE(USE_XEGL, 1, "xegl on")
AC_DEFINE(USE_GLES2, 1, "gles2 on")
AC_CONFIG_MACRO_DIR([build])
AC_CONFIG_AUX_DIR([build])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CXX
AC_PROG_LIBTOOL

AM_INIT_AUTOMAKE

CPPFLAGS="$CPPFLAGS -I/usr/local/include/sigc++-2.0 -I/usr/local/lib/sigc++-2.0/include"

GRAPEVINE_DEPENDENCIES="grapevine"
PKG_CHECK_MODULES(GRAPEVINE_DEPENDENCIES, "$GRAPEVINE_DEPENDENCIES")
AC_SUBST(GRAPEVINE_DEPENDENCIES_CFLAGS)
AC_SUBST(GRAPEVINE_DEPENDENCIES_LIBS)

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
