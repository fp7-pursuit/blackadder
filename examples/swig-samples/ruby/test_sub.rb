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

# Simple text subscriber example (blocking).

require 'blackadder'

ba = Blackadder::Blackadder::Instance(true)

sid = 0xa.chr + 0x0.chr*6 + 0xb.chr
rid = 0xc.chr + 0x0.chr*6 + 0xd.chr

ba.subscribe_info(rid, sid, Blackadder::NODE_LOCAL, nil)

while true
    ev = Blackadder::Event::new
    ba.getEvent(ev)
    p ev
    p ev.type
    if ev.type != Blackadder::UNDEF_EVENT
        p ev.id
        p ev.data_len
        p ev.data
    end
end

ba.disconnect()
