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

lib_LTLIBRARIES = libblackadder_c.la
libblackadder_c_la_SOURCES = blackadder_c.cc nb_blackadder_c.cc
libblackadder_c_la_LIBADD = ../../libblackadder.la
libblackadder_c_la_CXXFLAGS = $(DEBUGFLAGS)
libblackadder_c_la_LDFLAGS = -version-info $(MAJOR):$(MINOR) -pthread

include_HEADERS = blackadder_c.h nb_blackadder_c.h
