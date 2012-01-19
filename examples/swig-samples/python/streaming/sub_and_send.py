#!/usr/bin/env python

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

"""Subscribe to a stream of data and send it over a socket."""

from blackadder.blackadder import *
import socket

def _main(argv=[]):
    strategy = NODE_LOCAL # XXX: NODE_LOCAL=0, DOMAIN_LOCAL=2
    if len(argv) >= 2:
        strategy = int(argv[1])
    str_opt = None
    
    local_port = 0xDCAC
    if len(argv) >= 3:
        local_port = int(argv[2])
    
    ba = Blackadder_Instance(True)
    try:
        sid  = '\x0a'+6*'\x00'+'\x0b'
        rid  = '\x0c'+6*'\x00'+'\x0d'
        
        ba.subscribe_info(rid, sid, strategy, str_opt)
        
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        
        while True:
            ev = Event()
            ba.getEvent(ev)
            sock.sendto(ev.data, 0, ("localhost", local_port))
    finally:
        ba.disconnect()

if __name__ == "__main__":
    import sys
    _main(sys.argv)
