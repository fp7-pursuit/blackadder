/*
 * Copyright (C) 2010-2012  George Parisis and Dirk Trossen
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

#include "ns3/log.h"
#include "ns3/command-line.h"
#include "ns3/ptr.h"
#include "ns3/config.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"

#include "ns3/node.h"
#include "ns3/queue.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/point-to-point-net-device.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("BlackadderExample");

int 
main (int argc, char *argv[])
{
  LogComponentEnable ("BlackadderExample", LOG_LEVEL_INFO);
  Simulator::Destroy ();
}
