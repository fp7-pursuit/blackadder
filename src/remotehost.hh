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

#ifndef CLICK_REMOTEHOST_HH
#define CLICK_REMOTEHOST_HH

#include "helper.hh"
#include "common.hh"

/**
 * @brief (blackadder Core) The RemoteHost class represents a Blackadder network node.
 * 
 * A network node is identified using a statistically unique node label of size NODEID_LEN (see helper.hh).
 */
class RemoteHost {
public:
    /**
     * @brief Constructor:
     * @param _remoteHostID the statistically unique identifier of a Blackadder node.
     */
    RemoteHost(String _remoteHostID);
    /**@brief the statistically unique identifier of a Blackadder node.
     */
    String remoteHostID;
    /** @brief A set of String Items identifying published Scopes. The LocalRV uses this set.
     */
    StringSet publishedScopes;
    /** @brief A set of String Items identifying published Information Items. The LocalRV uses this set.
     */
    StringSet publishedInformationItems;
    /** @brief A set of String Items identifying Scope Subscriptions. The LocalRV uses this set.
     */
    StringSet subscribedScopes;
    /** @brief A set of String Items identifying InformationItem Subscriptions. The LocalRV uses this set.
     */
    StringSet subscribedInformationItems;
};

#endif
