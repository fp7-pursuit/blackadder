#-
# Copyright (C) 2012  Oy L M Ericsson Ab, NomadicLab
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

from blackadder.blackadder import Event as _blackadder_Event

def print_event(ev):
    print ev.type
    print repr(ev.id)
    print ev.data_len
    print repr(ev.data[:])

def get_and_print_event(ba):
        ev = _blackadder_Event()
        ba.getEvent(ev)
        print_event(ev)

def get_and_print_events(ba):
    while True:
        get_and_print_event(ba)
