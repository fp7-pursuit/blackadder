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

SWIGCXXFLAGS += -I$(PYTHON_INCDIR)

BUILT_SOURCES = blackadder_wrap.cxx

pkgpython_PYTHON = __init__.py blackadder_py.py blackadder.py
pkgpyexec_LTLIBRARIES = _blackadder_py.la
_blackadder_py_la_SOURCES = blackadder_wrap.cxx
_blackadder_py_la_LIBADD = $(LIBDIR)/libblackadder.la
_blackadder_py_la_CXXFLAGS = $(SWIGCXXFLAGS)
_blackadder_py_la_LDFLAGS = -module -version-info $(MAJOR):$(MINOR)
_blackadder_py_la_LDFLAGS += $(SWIGLDFLAGS)

SWIGFLAGS += -python -threads -outdir .

blackadder_wrap.cxx: *.i $(LIBDIR)/*.hpp
	$(SWIG) $(SWIGFLAGS) blackadder.i

clean-local:
	rm -f blackadder_wrap.cxx
	rm -f blackadder_py.py
	rm -f *.pyc *.pyo
