AC_DEFUN([XX_PATH_PYTHON], [
  if test -z "$PYTHON"; then
    AC_PATH_PROG([PYTHON], [python], [:])
  fi

  if test "$PYTHON" = :; then
    AC_MSG_ERROR([no Python interpreter found])
  else

    AC_CACHE_CHECK([for python include directory], [xx_cv_python_incdir],
      [xx_cv_python_incdir=`$PYTHON -c "import sys; from distutils import sysconfig; sys.stdout.write(sysconfig.get_python_inc())"`])
    AC_SUBST([PYTHON_INCDIR], [$xx_cv_python_incdir])

  fi

])

AC_DEFUN([XX_PATH_RUBY], [
  if test -z "$RUBY"; then
    AC_PATH_PROG([RUBY], [ruby], [:])
  fi

  if test "$RUBY" = :; then
    AC_MSG_ERROR([no Ruby interpreter found])
  else

    AC_CACHE_CHECK([for ruby include directory], [xx_cv_ruby_archdir],
      [xx_cv_ruby_archdir=`$RUBY -rrbconfig -e 'puts Config::CONFIG[["archdir"]]'`])
    AC_SUBST([RUBY_ARCHDIR], [$xx_cv_ruby_archdir])

    AC_CACHE_CHECK([for ruby architecture], [xx_cv_ruby_arch],
      [xx_cv_ruby_arch=`$RUBY -rrbconfig -e 'puts Config::CONFIG[["arch"]]'`])
    AC_SUBST([RUBY_ARCH], [$xx_cv_ruby_arch])

    AC_CACHE_CHECK([for ruby version], [xx_cv_ruby_ver],
      [xx_cv_ruby_ver=`$RUBY -rrbconfig -e 'puts Config::CONFIG[["ruby_version"]]'`])
    AC_SUBST([RUBY_VER], [$xx_cv_ruby_ver])

  fi

])
