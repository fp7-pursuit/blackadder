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

from blackadder_py import *

class _EventProxy(Event):
    def __init__(self, this):
        self.this = this

def blackadder_event_handler(func):
    """
    Wrap a function to be a Blackadder event handler.

    Example:
    @blackadder_event_handler
    def event_handler(ev):
        ...
    """
    def _event_handler(evptr):
        # Convert Event * to Event object. Call function.
        func(_EventProxy(evptr))
    return _event_handler

def evptr_to_obj(evptr):
    """Convert Event * to Event object."""
    return _EventProxy(evptr)
