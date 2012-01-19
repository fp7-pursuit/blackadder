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

"""Receive data over a socket and publish it."""

from blackadder.blackadder import *
from select import error
from select import select
from threading import Thread
from time import sleep
import os
import socket

def _main(argv=[]):
    strategy = NODE_LOCAL # XXX: NODE_LOCAL=0, DOMAIN_LOCAL=2
    if len(argv) >= 2:
        strategy = int(argv[1])
    str_opt = None
    
    local_port = 0xACDC
    if len(argv) >= 3:
        local_port = int(argv[2])
    
    global publishing # XXX
    publishing = False
    sock = None
    wfd, rfd = None, None
    
    ba = Blackadder.Instance(True)
    try:
        sid  = '\x0a'+6*'\x00'+'\x0b'
        rid  = '\x0c'+6*'\x00'+'\x0d'
        id   = sid + rid
        
        ba.publish_scope(sid, "", strategy, str_opt)
        ba.publish_info(rid, sid, strategy, str_opt)
        
        def recv_and_pub(sock, ba, rfd):
            try:
                sock.bind(("localhost", local_port)) # XXX
                sfd = sock.fileno()
                buf = rwbuffer(4096)
                while publishing:
                    rlist, wlist, xlist = select([sfd, rfd], [], [sfd, rfd])
                    if sfd in rlist:
                        n = sock.recv_into(buf)
                        ba.publish_data(id, strategy, str_opt,
                                        buffer(buf, 0, n))
                    if rfd in rlist:
                        print "received from pipe"
                        break
                    elif sfd in xlist or rfd in xlist:
                        print "exceptional condition"
                        break
            except error, select_error:
                # select() may raise an error if fds are closed.
                print "select:", select_error
            print "recv_and_pub stopped"
        
        while True:
            ev = Event()
            ba.getEvent(ev)
            print ev
            if not ev:
                continue
            print ev.type, "when", publishing
            if ev.type == START_PUBLISH and not publishing:
                print "Start publish"
                sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                rfd, wfd = os.pipe()
                t = Thread(target=recv_and_pub, args=[sock, ba, rfd])
                publishing = True
                t.start()
                print "Thread started"
                sleep(0.1) # XXX: Wait for thread to start.
            elif ev.type == STOP_PUBLISH and publishing:
                print "Stop publish"
                publishing = False
                os.write(wfd, '\0')
                sleep(0.1) # XXX: Wait for thread to stop.
                sock.close(); sock = None
                print "Socket closed"
                os.close(rfd); rfd = None
                os.close(wfd); wfd = None
                sleep(0.1) # XXX: Wait for thread to stop.
    finally:
        ba.disconnect()
        print "Blackadder disconnected"
        if sock:
            sock.close()
            print "Socket closed (finally)"
        if rfd:
            os.close(rfd)
        if wfd:
            os.close(wfd)

if __name__ == "__main__":
    import sys
    _main(sys.argv)
