#!/usr/bin/env ruby

#-
# Copyright (C) 2011-2012  Oy L M Ericsson Ab, NomadicLab
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

# Simple text publisher example.

require 'blackadder'

ba = Blackadder::NB_Blackadder::Instance(true)

sid = 0xa.chr + 0x0.chr*6 + 0xb.chr
rid = 0xc.chr + 0x0.chr*6 + 0xd.chr

class EventHandler
    def initialize(ba, sid, rid)
        @ba = ba
        @sid = sid
        @rid = rid
    end
    def event_handler(ev)
        p ev
        p ev.type
        if ev != nil && ev.type == Blackadder::START_PUBLISH
            data = Blackadder::to_malloc_buffer("test")
            @ba.publish_data(@sid+@rid, Blackadder::NODE_LOCAL, nil, data)
        end
    end
end
evh = EventHandler.new(ba, sid, rid)

ba.setRubyCallback(evh, 'event_handler')

ba.publish_scope(sid, "", Blackadder::NODE_LOCAL, nil)
ba.publish_info(rid, sid, Blackadder::NODE_LOCAL, nil)

ba.join()

ba.disconnect()
