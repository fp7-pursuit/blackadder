#!/usr/bin/env python

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

"""Channel publisher example."""

from blackadder.blackadder import *

def _main(argv=[]):
    kernel = False
    if len(argv) >= 2:
        kernel = bool(int(argv[1]))

    strategy = DOMAIN_LOCAL # Our default strategy is domain-local
    if len(argv) >= 3:
        strategy = int(argv[2])
    
    ba = Blackadder.Instance(not kernel)
    
    sid0 = ""
    sid  = PURSUIT_ID_LEN*'\x11'
    rid  = sid
    
    psize = 1400;
    p1 = psize * 'A'
    p2 = psize * 'B'
    n1 = 1000000;
    n2 = 100;
    
    ba.publish_scope(sid, sid0, strategy, None)
    ba.publish_info(rid, sid, strategy, None)
    
    try:
        while True:
            ev = Event()
            ba.getEvent(ev)
            print ev
            if ev and ev.type == START_PUBLISH:
                break
        
        for i in xrange(n1):
            ba.publish_data(sid+rid, strategy, None, buffer(p1))
        for i in xrange(n2):
            ba.publish_data(sid+rid, strategy, None, buffer(p2))
    finally:
        ba.disconnect()

if __name__ == "__main__":
    import os; print "PID =", os.getpid()
    import sys
    _main(sys.argv)
