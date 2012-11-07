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

"""Channel subscriber example."""

from blackadder.blackadder import *
import time

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
    
    psize = 1400;
    c2 = 'B'
    n1 = 0
    t1 = 0
    
    print "Subscribing"
    ba.subscribe_scope(sid, sid0, strategy, None)
    
    ev = Event()
    try:
        buf = rwbuffer(psize) # Reusing this buffer for events
        while True:
            ba.getEventIntoBuf(ev, buf)
            if ev and ev.type == PUBLISHED_DATA:
                if n1 == 0:
                    t1 = time.time()
                if ev.data and ev.data[0] == c2:
                    t2 = time.time()
                    print_stats(psize, n1, t1, t2)
                    break
                n1 += 1
    finally:
        ev.buffer = None # Let Python free buf instead
        ba.disconnect()

def print_stats(psize, n, t1, t2):
    t = t2 - t1
    b = n * psize
    print "%d packets" % n
    print "%d bytes (%d)" % (b, psize)
    print "%d seconds" % t
    mb = (float(b) / (1024**2)) / t
    print "%d MB/s" % mb
    pps = float(n) / t
    print "%d pps" % pps
    
if __name__ == "__main__":
    import os;  print "PID =", os.getpid()
    import sys
    _main(sys.argv)
