#!/usr/bin/env python

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

"""Very simple non-blocking text publisher example."""

from blackadder.blackadder import *

def _main(argv=[]):
    strategy = NODE_LOCAL
    if len(argv) >= 2:
        strategy = int(argv[1])
    
    ba = NB_Blackadder.Instance(True)

    @blackadder_event_handler
    def event_handler(ev):
        print ev
        if not ev or ev.type != START_PUBLISH:
            return
        data = "Hello"
        ba.publish_data(sid+rid, strategy, None, to_malloc_buffer(data))
    
    ba.setPyCallback(event_handler)
    
    sid0 = ""
    sid  = '\x0a'+6*'\x00'+'\x0b'
    rid  = '\x0c'+6*'\x00'+'\x0d'
    
    ba.publish_scope(sid, sid0, strategy, None)
    ba.publish_info(rid, sid, strategy, None)
    
    try:
        ba.join()
    finally:
        ba.disconnect()

if __name__ == "__main__":
    import sys
    _main(sys.argv)
