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

"""Read data and send it over a socket."""
# Can be used together with recv_and_pub.

from socket import *

def _main(argv=[]):
    local_port = 0xACDC
    if len(argv) >= 2:
        local_port = int(argv[1])
    
    sock = socket(AF_INET, SOCK_DGRAM)
    while True:
        print "Whaddaya wanna send?"
        data = raw_input()
        print data
        sock.sendto(data, 0, ("localhost", local_port))

if __name__ == "__main__":
    import sys
    _main(sys.argv)
