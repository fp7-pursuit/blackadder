#-
# Copyright (C) 2011  Oy L M Ericsson Ab, NomadicLab
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# Alternatively, this software may be distributed under the terms of the
# BSD license.
#
# See LICENSE and COPYING for more details.
#

include ../swig-common.am
include includes.am

JAVAC ?= javac
SWIGCXXFLAGS += $(JAVA_INCLUDES)

BUILT_SOURCES = blackadder_wrap.cxx

lib_LTLIBRARIES = libblackadder_java.la
libblackadder_java_la_SOURCES = blackadder_wrap.cxx
libblackadder_java_la_LIBADD = $(LIBDIR)/libblackadder.la
libblackadder_java_la_CXXFLAGS = $(SWIGCXXFLAGS)
libblackadder_java_la_LDFLAGS = -version-info $(MAJOR):$(MINOR)
libblackadder_java_la_LDFLAGS += $(SWIGLDFLAGS)

SWIG_PACKAGE = blackadder_java
SWIG_OUTDIR = $(SWIG_PACKAGE).src
SWIGFLAGS += -java -outdir $(SWIG_OUTDIR) -package $(SWIG_PACKAGE)

blackadder_wrap.cxx: *.i $(LIBDIR)/*.hpp
	test -d $(SWIG_OUTDIR) || mkdir -p $(SWIG_OUTDIR)
	$(SWIG) $(SWIGFLAGS) blackadder.i
	$(JAVAC) $(SWIG_OUTDIR)/*.java -d .

clean-local:
	rm -f blackadder_wrap.cxx
	rm -rf $(SWIG_OUTDIR)
	rm -rf $(SWIG_PACKAGE)

install-data-hook:
	cp -r $(SWIG_PACKAGE) $(libdir)

uninstall-hook:
	test -n "$(SWIG_PACKAGE)" && rm -rf $(libdir)/$(SWIG_PACKAGE)

# XXX: No success so far with putting the package into a jar-file (JNI
#      functions from the library not accessible in that case).
