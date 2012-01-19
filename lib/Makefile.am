NAMES = blackadder nb_blackadder bitvector
HDRS = $(NAMES:=.hpp)
SRCS = $(NAMES:=.cpp)

#DEBUGFLAGS = -ggdb

lib_LTLIBRARIES = libblackadder.la
libblackadder_la_SOURCES = $(SRCS)
libblackadder_la_CXXFLAGS = $(DEBUGFLAGS)
libblackadder_la_LDFLAGS = -version-info $(MAJOR):$(MINOR)

include_HEADERS = $(HDRS) blackadder_defs.h

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = . swig bindings

# XXX: Remove old libblackadder.a in install and uninstall hooks

install-exec-hook:
	test "$(libdir)" != "/usr/lib" -a -e /usr/lib/libblackadder.a && rm -fv /usr/lib/libblackadder.a || true # XXX
	test "$(includedir)" != "/usr/include" -a -e /usr/include/blackadder.hpp && rm -fv /usr/include/blackadder.hpp || true # XXX
	test "$(includedir)" != "/usr/include" -a -e /usr/include/nb_blackadder.hpp && rm -fv /usr/include/nb_blackadder.hpp || true # XXX
	test "$(includedir)" != "/usr/include" -a -e /usr/include/bitvector.hpp && rm -fv  /usr/include/bitvector.hpp || true # XXX
	test -n "$(LDCONFIG)" -a "$(libdir)" = "/usr/local/lib" && $(LDCONFIG) /usr/local/lib || true # XXX

uninstall-hook:
	test "$(libdir)" != "/usr/lib" -a -e /usr/lib/libblackadder.a && rm -fv /usr/lib/libblackadder.a || true # XXX
	test "$(includedir)" != "/usr/include" -a -e /usr/include/blackadder.hpp && rm -fv /usr/include/blackadder.hpp || true # XXX
	test "$(includedir)" != "/usr/include" -a -e /usr/include/nb_blackadder.hpp && rm -fv /usr/include/nb_blackadder.hpp || true # XXX
	test "$(includedir)" != "/usr/include" -a -e /usr/include/bitvector.hpp && rm -fv /usr/include/bitvector.hpp || true # XXX
