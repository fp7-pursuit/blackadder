#!/usr/bin/env python

#-
# Copyright (C) 2013  Oy L M Ericsson Ab, NomadicLab
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

"""Test data publisher."""

from blackadder.blackadder import *
import time

ba = [None]

def pub_scope(prefix, sid):
    ba[0].publish_scope(sid, prefix, NODE_LOCAL, None)
    time.sleep(0.01)

def pub_info(prefix, rid):
    ba[0].publish_info(rid, prefix, NODE_LOCAL, None)
    time.sleep(0.01)

def sub_scope(prefix, sid):
    ba[0].subscribe_scope(sid, prefix, NODE_LOCAL, None)
    time.sleep(0.01)

def sub_info(prefix, rid):
    ba[0].subscribe_info(rid, prefix, NODE_LOCAL, None)
    time.sleep(0.01)

def publish():
    ba[0] = Blackadder.Instance(True)
    try:
        sids = [PURSUIT_ID_LEN*chr(i) for i in xrange(0, 0x7f+1)]
        rids = [PURSUIT_ID_LEN*chr(i) for i in xrange(0x80, 0xff+1)]
    
        pub_scope("", sids[0])
        pub_scope(sids[0], sids[1])
        pub_scope(sids[0], sids[2])
        
        pub_info(sids[0]+sids[1], rids[0])
        pub_info(sids[0]+sids[2], rids[0])
        
        pub_info(sids[0]+sids[1], rids[1])
        pub_info(sids[0]+sids[2], sids[0]+sids[1]+rids[1])

        pub_info(sids[0], rids[3])
        #pub_info("", rids[4]) # Error
        pub_info(sids[0]+sids[1], sids[0])

        pub_scope(sids[0]+sids[1], sids[0]+sids[2])
        pub_info(sids[0]+sids[2], "PRINTABL")
        pub_info(sids[0]+sids[1], "PRINT\x00\x00\x01")

        pub_scope("", sids[1])
        pub_info(sids[1], sids[0]+rids[3])

        sub_scope("", sids[0])
        sub_info(sids[0]+sids[2], rids[1])
        sub_info(sids[0]+sids[1], rids[1])
    finally:
#        ba[0].disconnect()
        pass

def _main(argv):
    publish()

if __name__ == "__main__":
    import sys
    _main(sys.argv)
