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

SWIGCXXFLAGS += -I$(RUBY_ARCHDIR)

if DARWIN
SWIGLDFLAGS += -shrext .bundle -Wl,-bundle -Wl,-undefined -Wl,dynamic_lookup
endif

BUILT_SOURCES = blackadder_wrap.cxx

blackadderdir = $(libdir)/ruby/site_ruby/$(RUBY_VER)/$(RUBY_ARCH)

blackadder_LTLIBRARIES = blackadder.la
blackadder_la_SOURCES = blackadder_wrap.cxx
blackadder_la_LIBADD = $(LIBDIR)/libblackadder.la
blackadder_la_CXXFLAGS = $(SWIGCXXFLAGS)
blackadder_la_LDFLAGS = -module -version-info $(MAJOR):$(MINOR)
blackadder_la_LDFLAGS += $(SWIGLDFLAGS)

SWIGFLAGS += -ruby -outdir .

blackadder_wrap.cxx: *.i $(LIBDIR)/*.hpp
	$(SWIG) $(SWIGFLAGS) blackadder.i

clean-local:
	rm -f blackadder_wrap.cxx
