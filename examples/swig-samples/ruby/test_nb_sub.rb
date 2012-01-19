#!/usr/bin/env ruby

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

# Simple text subscriber example (non-blocking).

require 'blackadder'

ba = Blackadder::NB_Blackadder::Instance(true)

sid = 0xa.chr + 0x0.chr*6 + 0xb.chr
rid = 0xc.chr + 0x0.chr*6 + 0xd.chr

class EventHandler
    def event_handler(ev)
        p ev
        p ev.type
    end
end
evh = EventHandler.new

ba.setRubyCallback(evh, 'event_handler')

ba.subscribe_info(rid, sid, Blackadder::NODE_LOCAL, nil)

ba.join()

ba.disconnect()
