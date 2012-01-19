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

"""Receive data over a socket a print it."""
# Can be used together with sub_and_send.

from socket import *

def _main(argv=[]):
    local_port = 0xDCAC
    if len(argv) >= 2:
        local_port = int(argv[1])
    
    sock = socket(AF_INET, SOCK_DGRAM)
    sock.bind(("localhost", local_port))
    while True:
        data = sock.recv(4096)
        print "Received:"
        print data

if __name__ == "__main__":
    import sys
    _main(sys.argv)
