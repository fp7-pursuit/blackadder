# Copyright (C) 2012, Oy L M Ericsson Ab, NomadicLab. All rights reserved.

SUBDIRS1=src lib
SUBDIRS2=TopologyManager deployment examples/samples
SUBDIRS=$(SUBDIRS1) $(SUBDIRS2)

.PHONY: all subdirs $(SUBDIRS) clean distclean install uninstall configure \
	install-deps

all: subdirs

subdirs: $(SUBDIRS)

base: $(SUBDIRS1)

apps: $(SUBDIRS2)

$(SUBDIRS):
	$(MAKE) -C $@

clean:: $(addsuffix -clean,$(SUBDIRS))

%-clean::
	$(MAKE) -C $* clean

distclean:: $(addsuffix -distclean,$(SUBDIRS1))

%-distclean::
	$(MAKE) -C $* distclean

install:: $(addsuffix -install,$(SUBDIRS1))

%-install::
	$(MAKE) -C $* install

uninstall:: $(addsuffix -uninstall,$(SUBDIRS1))

%-uninstall::
	$(MAKE) -C $* uninstall

configure:: $(addsuffix -configure,$(SUBDIRS1))

%-configure::
	cd $*; ./configure $(CONFIGFLAGS); cd ..

install-deps:
	apt-get install `cat apt-get.txt`
