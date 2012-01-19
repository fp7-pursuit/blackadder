#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Autoconf initialisation.
AC_PREREQ([2.67])
AC_INIT([blackadder], [0.2], [pursuit-code@hiit.fi])
AC_SUBST(MAJOR, 0)
AC_SUBST(MINOR, 2)
#AC_CONFIG_SRCDIR([blackadder.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])

# Automake Initialisation.
AM_INIT_AUTOMAKE([gnu])

# Turn on processing for libtool.
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

AM_PROG_CC_C_O

dnl # Checks for libraries.
dnl AC_CHECK_LIB([m], [acos])

dnl # Checks for header files.
dnl AC_CHECK_HEADERS([fcntl.h float.h inttypes.h limits.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h])

dnl # Checks for typedefs, structures, and compiler characteristics.
dnl AC_HEADER_STDBOOL
dnl AC_C_INLINE
dnl AC_TYPE_SIZE_T
dnl AC_TYPE_SSIZE_T
dnl AC_TYPE_UINT16_T
dnl AC_TYPE_UINT32_T

dnl # Checks for library functions.
dnl AC_FUNC_MALLOC
dnl AC_CHECK_FUNCS([floor memset select socket strstr])

HOSTOS=unknown
case "${host}" in
    *-*-linux*)   HOSTOS=linux ;;
    *-*-freebsd*) HOSTOS=freebsd ;;
    *-*-darwin*)  HOSTOS=darwin ;;
esac
AM_CONDITIONAL(LINUX,   test $HOSTOS = linux)
AM_CONDITIONAL(FREEBSD, test $HOSTOS = freebsd)
AM_CONDITIONAL(DARWIN,  test $HOSTOS = darwin)
AC_SUBST(HOSTOS)

AC_PATH_PROG([LDCONFIG], [ldconfig])

AC_PATH_PROG([SWIG], [swig], [swig]) # XXX

AC_ARG_ENABLE(python, [  --enable-python         enable Python bindings (EXPERIMENTAL)], :, enable_python=no)
AM_CONDITIONAL(SWIG_PYTHON, test x$enable_python = xyes)
AC_ARG_ENABLE(ruby,   [  --enable-ruby           enable Ruby bindings (EXPERIMENTAL)], :, enable_ruby=no)
AM_CONDITIONAL(SWIG_RUBY,   test x$enable_ruby = xyes)
AC_ARG_ENABLE(java,   [  --enable-java           enable Java bindings (EXPERIMENTAL)], :, enable_java=no)
AM_CONDITIONAL(SWIG_JAVA,   test x$enable_java = xyes)

if test "x$enable_python" = xyes; then
    AM_PATH_PYTHON
    XX_PATH_PYTHON
fi

if test "x$enable_ruby" = xyes; then
    XX_PATH_RUBY
fi

if test "x$enable_java" = xyes; then
    AC_PATH_PROG([JAVAC], [javac], [javac]) # XXX
fi

AC_ARG_ENABLE(c, [  --enable-c              enable C bindings (EXPERIMENTAL)], :, enable_c=no)
AM_CONDITIONAL(C_BINDING, test x$enable_c = xyes)

AC_CONFIG_FILES([Makefile
                 swig/Makefile
                 swig/python/Makefile
                 swig/ruby/Makefile
                 swig/java/Makefile
                 bindings/Makefile
                 bindings/c-binding/Makefile])
AC_OUTPUT
