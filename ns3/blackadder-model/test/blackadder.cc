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

#include "ns3/test.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/simulator.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/point-to-point-channel.h"

namespace ns3 {

class BlackadderTest : public TestCase
{
public:
  BlackadderTest ();

  virtual void DoRun (void);
};

BlackadderTest::BlackadderTest (): TestCase ("BlackadderTest")
{
}

void
BlackadderTest::DoRun (void)
{
  Simulator::Run ();

  Simulator::Destroy ();
}
//-----------------------------------------------------------------------------
class BlackadderTestSuite : public TestSuite
{
public:
  BlackadderTestSuite ();
};

BlackadderTestSuite::BlackadderTestSuite (): TestSuite ("blackadder test suite", UNIT)
{
  AddTestCase (new BlackadderTest);
}

static BlackadderTestSuite g_blackadderTestSuite;

} // namespace ns3


