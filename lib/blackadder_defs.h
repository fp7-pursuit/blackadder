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

#ifndef BLACKADDER_DEFS_HPP
#define BLACKADDER_DEFS_HPP

/**********************************/
#define PURSUIT_ID_LEN 8 //in bytes
#define FID_LEN 32 //in bytes
#define NODEID_LEN PURSUIT_ID_LEN //in bytes
/****some strategies*****/
#define NODE_LOCAL          0
#define LINK_LOCAL          1
#define DOMAIN_LOCAL        2
#define IMPLICIT_RENDEZVOUS 3
#define BROADCAST_IF        4
/************************/
/*intra and inter click message types*/
#define PUBLISH_SCOPE 0
#define PUBLISH_INFO 1
#define UNPUBLISH_SCOPE 2
#define UNPUBLISH_INFO 3
#define SUBSCRIBE_SCOPE 4
#define SUBSCRIBE_INFO 5
#define UNSUBSCRIBE_SCOPE 6
#define UNSUBSCRIBE_INFO 7
#define PUBLISH_DATA  8 //the request
#define CONNECT 12
#define DISCONNECT 13
/*****************************/
#define UNDEF_EVENT 0
#define START_PUBLISH 100
#define STOP_PUBLISH 101
#define SCOPE_PUBLISHED 102
#define SCOPE_UNPUBLISHED 103
#define PUBLISHED_DATA 104
#define MATCH_PUB_SUBS 105
#define RV_RESPONSE 106	
#define NETLINK_BADDER 20

#endif /* BLACKADDER_DEFS_HPP */
