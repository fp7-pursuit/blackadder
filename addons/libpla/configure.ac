#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License version
# 2 as published by the Free Software Foundation.
#

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT(libpla, 0.2)
AC_PREREQ(2.57)

AM_INIT_AUTOMAKE
AC_PROG_CXX
AC_PROG_LIBTOOL

AC_CONFIG_MACRO_DIR([m4])

AM_PROG_CC_C_O

# check for GMP
AC_CHECK_LIB(gmp, __gmpz_init, , [AC_MSG_ERROR([GNU MP not found, see http://swox.com/gmp/])])

AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(src/Makefile)
AC_OUTPUT
