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
#include "activepub.hh"

CLICK_DECLS

ActivePublication::ActivePublication(String _fullID, unsigned char _strategy, bool _isScope) {
    fullID = _fullID;
    strategy = _strategy;
    isScope = _isScope;
}

ActivePublication::~ActivePublication() {
}

ActiveSubscription::ActiveSubscription(String _fullID, unsigned char _strategy, bool _isScope) {
    fullID = _fullID;
    strategy = _strategy;
    isScope = _isScope;
}

ActiveSubscription::~ActiveSubscription() {
}

CLICK_ENDDECLS
ELEMENT_PROVIDES(ActivePublication)
ELEMENT_PROVIDES(ActiveSubscription)
