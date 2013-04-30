/*
 * Copyright (C) 2012-2013  Andreas Bontozoglou
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
#ifndef QOS_STRUCTS_HPP
#define QOS_STRUCTS_HPP

#include <map>
#include <inttypes.h>

/// Network Type Values
#define NT_NULL		0
#define NT_ETH		1
#define NT_802_11n	2
#define NT_802_11g	3
#define NT_802_11b	4
#define NT_802_11a	5
#define NT_802_16	6
#define NT_3G		7
#define NT_PPP		8
#define NT_DSL		9
#define NT_OVPN		10
#define NT_L2TP		11

/// Flow Attr MASKS
#define ATTR_NULL	0
#define ATTR_BE		1
#define ATTR_EF		2
#define ATTR_AF		4
#define ATTR_NO_CACHE	8


/// All the types of QoS elements. Extend ass needed.
/// NULL Item
#define QoS_NULL	0
/// Link capacity (Mbps)
#define QoS_CAPACITY_M	1
/// Link capacity (Kbps)
#define QoS_CAPACITY_K	2

/// Queue length (packets)
#define QoS_QUEUELEN	4
/// Round Trip Time (ns)
#define QoS_RTT		5
/// Jitter (ns)
#define QoS_JITTER	6
/// Delay (ms)
#define QoS_DELAY	7
/// Signal to Noise 0-100
#define QoS_SNR		8
/// FIXME:Signal to Interference 0-100
#define QoS_SINR	9
/// Network type
#define QoS_NETTYPE	10
/// Incoming Data rate (Mbps)
#define QoS_INRATE_M	11
/// Ocupied Data rate (Mbps)
#define QoS_OUTRATE_M	12
/// Incoming Data rate (Kbps)
#define QoS_INRATE_K	13
/// Ocupied Data rate (Kbps)
#define QoS_OUTRATE_K	14
/// The link priority (may or may not change dynamically: currently it doesn't)
#define QoS_PRIO	15
/// The current BS limit (Kbps)
#define QoS_BS_LIM_K	16
/// The current BS limit (Mbps)
#define QoS_BS_LIM_M	17

/// QoS items for metadata (traffic spec.)
/// Bandiwdth/data rate needed (mean)
#define QoS_BW_K	50
#define QoS_BW_M	51
/// More information/characteristics about the flow (ie: cache/no-cache)
#define QoS_FLOW	52
/// Avg. packet size
#define QoS_PKTSIZE	53
/// Max. Time of a burst (ms)
#define QoS_BURSTSIZE	54
/// Peak rate on a burst
#define QoS_PEAKRATE_K	55
#define QoS_PEAKRATE_M	56
/// Delagate: If the metadata of this item delegate to the children
#define QoS_DELEGATE	57

/**
 * Define a QoS list in the form of key-value
 */
typedef std::map<uint8_t, uint16_t> QoSList;
#define QoS_ITEM_SIZE	3

/// Maximum link priority (needed to calculate cost)
#define MAX_PRIO 100

/// Link state monitoring scope. Defined here cause apps may need to monitor
/// links (ie DBA apps, or Network Status Display apps).
/// XXX: Doesn't take PURSUIT_ID_LEN into consideration.
#define LSM_SCOPE "fffffffffffffffb"

#endif
