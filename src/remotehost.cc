/*
* Copyright (C) 2010-2011  George Parisis and Dirk Trossen
* All rights reserved.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License version
* 2 as published by the Free Software Foundation.
*
* Alternatively, this software may be distributed under the terms of
* the BSD license.
*
* See LICENSE and COPYING for more details.
*/
#include "remotehost.hh"

CLICK_DECLS

RemoteHost::RemoteHost(String _remoteHostID) {
    remoteHostID = _remoteHostID;
}

CLICK_ENDDECLS
ELEMENT_PROVIDES(RemoteHost)