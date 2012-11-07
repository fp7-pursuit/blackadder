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

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/point-to-point-module.h>
#include <ns3/blackadder-module.h>

#include "publisher.h"
#include "subscriber.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("topology");

int main(int argc, char *argv[]) {
    Ptr<Node> node0 = CreateObject<Node > ();
    Ptr<PointToPointNetDevice> dev0_0 = Create<PointToPointNetDevice > ();
    dev0_0->SetAddress(Mac48Address("00:00:00:00:00:01"));
    dev0_0->SetDataRate(DataRate("100Mbps"));
    dev0_0->SetMtu(1500);
    node0->AddDevice(dev0_0);
    Ptr<DropTailQueue> queue0_0 = CreateObject<DropTailQueue > ();
    dev0_0->SetQueue(queue0_0);

    Ptr<Node> node1 = CreateObject<Node > ();
    Ptr<PointToPointNetDevice> dev1_0 = Create<PointToPointNetDevice > ();
    dev1_0->SetAddress(Mac48Address("00:00:00:00:00:02"));
    dev1_0->SetDataRate(DataRate("100Mbps"));
    dev1_0->SetMtu(1500);
    node1->AddDevice(dev1_0);
    Ptr<DropTailQueue> queue1_0 = CreateObject<DropTailQueue > ();
    dev1_0->SetQueue(queue1_0);
    Ptr<PointToPointNetDevice> dev1_1 = Create<PointToPointNetDevice > ();
    dev1_1->SetAddress(Mac48Address("00:00:00:00:00:03"));
    dev1_1->SetDataRate(DataRate("100Mbps"));
    dev1_1->SetMtu(1500);
    node1->AddDevice(dev1_1);
    Ptr<DropTailQueue> queue1_1 = CreateObject<DropTailQueue > ();
    dev1_1->SetQueue(queue1_1);
    Ptr<PointToPointNetDevice> dev1_2 = Create<PointToPointNetDevice > ();
    dev1_2->SetAddress(Mac48Address("00:00:00:00:00:05"));
    dev1_2->SetDataRate(DataRate("100Mbps"));
    dev1_2->SetMtu(1500);
    node1->AddDevice(dev1_2);
    Ptr<DropTailQueue> queue1_2 = CreateObject<DropTailQueue > ();
    dev1_2->SetQueue(queue1_2);
    Ptr<PointToPointNetDevice> dev1_3 = Create<PointToPointNetDevice > ();
    dev1_3->SetAddress(Mac48Address("00:00:00:00:00:07"));
    dev1_3->SetDataRate(DataRate("100Mbps"));
    dev1_3->SetMtu(1500);
    node1->AddDevice(dev1_3);
    Ptr<DropTailQueue> queue1_3 = CreateObject<DropTailQueue > ();
    dev1_3->SetQueue(queue1_3);

    Ptr<Node> node2 = CreateObject<Node > ();
    Ptr<PointToPointNetDevice> dev2_0 = Create<PointToPointNetDevice > ();
    dev2_0->SetAddress(Mac48Address("00:00:00:00:00:04"));
    dev2_0->SetDataRate(DataRate("100Mbps"));
    dev2_0->SetMtu(1500);
    node2->AddDevice(dev2_0);
    Ptr<DropTailQueue> queue2_0 = CreateObject<DropTailQueue > ();
    dev2_0->SetQueue(queue2_0);
    Ptr<PointToPointNetDevice> dev2_1 = Create<PointToPointNetDevice > ();
    dev2_1->SetAddress(Mac48Address("00:00:00:00:00:09"));
    dev2_1->SetDataRate(DataRate("100Mbps"));
    dev2_1->SetMtu(1500);
    node2->AddDevice(dev2_1);
    Ptr<DropTailQueue> queue2_1 = CreateObject<DropTailQueue > ();
    dev2_1->SetQueue(queue2_1);
    Ptr<PointToPointNetDevice> dev2_2 = Create<PointToPointNetDevice > ();
    dev2_2->SetAddress(Mac48Address("00:00:00:00:00:0b"));
    dev2_2->SetDataRate(DataRate("100Mbps"));
    dev2_2->SetMtu(1500);
    node2->AddDevice(dev2_2);
    Ptr<DropTailQueue> queue2_2 = CreateObject<DropTailQueue > ();
    dev2_2->SetQueue(queue2_2);
    Ptr<PointToPointNetDevice> dev2_3 = Create<PointToPointNetDevice > ();
    dev2_3->SetAddress(Mac48Address("00:00:00:00:00:0d"));
    dev2_3->SetDataRate(DataRate("100Mbps"));
    dev2_3->SetMtu(1500);
    node2->AddDevice(dev2_3);
    Ptr<DropTailQueue> queue2_3 = CreateObject<DropTailQueue > ();
    dev2_3->SetQueue(queue2_3);

    Ptr<Node> node3 = CreateObject<Node > ();
    Ptr<PointToPointNetDevice> dev3_0 = Create<PointToPointNetDevice > ();
    dev3_0->SetAddress(Mac48Address("00:00:00:00:00:06"));
    dev3_0->SetDataRate(DataRate("100Mbps"));
    dev3_0->SetMtu(1500);
    node3->AddDevice(dev3_0);
    Ptr<DropTailQueue> queue3_0 = CreateObject<DropTailQueue > ();
    dev3_0->SetQueue(queue3_0);
    Ptr<PointToPointNetDevice> dev3_1 = Create<PointToPointNetDevice > ();
    dev3_1->SetAddress(Mac48Address("00:00:00:00:00:0a"));
    dev3_1->SetDataRate(DataRate("100Mbps"));
    dev3_1->SetMtu(1500);
    node3->AddDevice(dev3_1);
    Ptr<DropTailQueue> queue3_1 = CreateObject<DropTailQueue > ();
    dev3_1->SetQueue(queue3_1);
    Ptr<PointToPointNetDevice> dev3_2 = Create<PointToPointNetDevice > ();
    dev3_2->SetAddress(Mac48Address("00:00:00:00:00:0f"));
    dev3_2->SetDataRate(DataRate("100Mbps"));
    dev3_2->SetMtu(1500);
    node3->AddDevice(dev3_2);
    Ptr<DropTailQueue> queue3_2 = CreateObject<DropTailQueue > ();
    dev3_2->SetQueue(queue3_2);

    Ptr<Node> node4 = CreateObject<Node > ();
    Ptr<PointToPointNetDevice> dev4_0 = Create<PointToPointNetDevice > ();
    dev4_0->SetAddress(Mac48Address("00:00:00:00:00:0c"));
    dev4_0->SetDataRate(DataRate("100Mbps"));
    dev4_0->SetMtu(1500);
    node4->AddDevice(dev4_0);
    Ptr<DropTailQueue> queue4_0 = CreateObject<DropTailQueue > ();
    dev4_0->SetQueue(queue4_0);

    Ptr<Node> node5 = CreateObject<Node > ();
    Ptr<PointToPointNetDevice> dev5_0 = Create<PointToPointNetDevice > ();
    dev5_0->SetAddress(Mac48Address("00:00:00:00:00:10"));
    dev5_0->SetDataRate(DataRate("100Mbps"));
    dev5_0->SetMtu(1500);
    node5->AddDevice(dev5_0);
    Ptr<DropTailQueue> queue5_0 = CreateObject<DropTailQueue > ();
    dev5_0->SetQueue(queue5_0);

    Ptr<Node> node6 = CreateObject<Node > ();
    Ptr<PointToPointNetDevice> dev6_0 = Create<PointToPointNetDevice > ();
    dev6_0->SetAddress(Mac48Address("00:00:00:00:00:0e"));
    dev6_0->SetDataRate(DataRate("100Mbps"));
    dev6_0->SetMtu(1500);
    node6->AddDevice(dev6_0);
    Ptr<DropTailQueue> queue6_0 = CreateObject<DropTailQueue > ();
    dev6_0->SetQueue(queue6_0);

    Ptr<Node> node7 = CreateObject<Node > ();
    Ptr<PointToPointNetDevice> dev7_0 = Create<PointToPointNetDevice > ();
    dev7_0->SetAddress(Mac48Address("00:00:00:00:00:08"));
    dev7_0->SetDataRate(DataRate("100Mbps"));
    dev7_0->SetMtu(1500);
    node7->AddDevice(dev7_0);
    Ptr<DropTailQueue> queue7_0 = CreateObject<DropTailQueue > ();
    dev7_0->SetQueue(queue7_0);

    Ptr<PointToPointChannel> channel0 = CreateObject<PointToPointChannel > ();
    channel0->SetAttribute("Delay", StringValue("10ms"));
    dev0_0->Attach(channel0);
    dev1_0->Attach(channel0);

    Ptr<PointToPointChannel> channel1 = CreateObject<PointToPointChannel > ();
    channel1->SetAttribute("Delay", StringValue("10ms"));
    dev1_1->Attach(channel1);
    dev2_0->Attach(channel1);

    Ptr<PointToPointChannel> channel2 = CreateObject<PointToPointChannel > ();
    channel2->SetAttribute("Delay", StringValue("10ms"));
    dev1_2->Attach(channel2);
    dev3_0->Attach(channel2);

    Ptr<PointToPointChannel> channel3 = CreateObject<PointToPointChannel > ();
    channel3->SetAttribute("Delay", StringValue("10ms"));
    dev1_3->Attach(channel3);
    dev7_0->Attach(channel3);

    Ptr<PointToPointChannel> channel4 = CreateObject<PointToPointChannel > ();
    channel4->SetAttribute("Delay", StringValue("10ms"));
    dev2_1->Attach(channel4);
    dev3_1->Attach(channel4);

    Ptr<PointToPointChannel> channel5 = CreateObject<PointToPointChannel > ();
    channel5->SetAttribute("Delay", StringValue("10ms"));
    dev2_2->Attach(channel5);
    dev4_0->Attach(channel5);

    Ptr<PointToPointChannel> channel6 = CreateObject<PointToPointChannel > ();
    channel6->SetAttribute("Delay", StringValue("10ms"));
    dev2_3->Attach(channel6);
    dev6_0->Attach(channel6);

    Ptr<PointToPointChannel> channel7 = CreateObject<PointToPointChannel > ();
    channel7->SetAttribute("Delay", StringValue("10ms"));
    dev3_2->Attach(channel7);
    dev5_0->Attach(channel7);

    Ptr<ClickBridge> click0 = CreateObject<ClickBridge > ();
    node0->AggregateObject(click0);
    click0->SetClickFile("/tmp//00000001.conf");
    Ptr<ServiceModel> servModel0 = CreateObject<ServiceModel > ();
    node0->AggregateObject(servModel0);
    Ptr<ClickBridge> click1 = CreateObject<ClickBridge > ();
    node1->AggregateObject(click1);
    click1->SetClickFile("/tmp//00000002.conf");
    Ptr<ServiceModel> servModel1 = CreateObject<ServiceModel > ();
    node1->AggregateObject(servModel1);
    Ptr<ClickBridge> click2 = CreateObject<ClickBridge > ();
    node2->AggregateObject(click2);
    click2->SetClickFile("/tmp//00000003.conf");
    Ptr<ServiceModel> servModel2 = CreateObject<ServiceModel > ();
    node2->AggregateObject(servModel2);
    Ptr<ClickBridge> click3 = CreateObject<ClickBridge > ();
    node3->AggregateObject(click3);
    click3->SetClickFile("/tmp//00000004.conf");
    Ptr<ServiceModel> servModel3 = CreateObject<ServiceModel > ();
    node3->AggregateObject(servModel3);
    Ptr<ClickBridge> click4 = CreateObject<ClickBridge > ();
    node4->AggregateObject(click4);
    click4->SetClickFile("/tmp//00000005.conf");
    Ptr<ServiceModel> servModel4 = CreateObject<ServiceModel > ();
    node4->AggregateObject(servModel4);
    Ptr<ClickBridge> click5 = CreateObject<ClickBridge > ();
    node5->AggregateObject(click5);
    click5->SetClickFile("/tmp//00000006.conf");
    Ptr<ServiceModel> servModel5 = CreateObject<ServiceModel > ();
    node5->AggregateObject(servModel5);
    Ptr<ClickBridge> click6 = CreateObject<ClickBridge > ();
    node6->AggregateObject(click6);
    click6->SetClickFile("/tmp//00000007.conf");
    Ptr<ServiceModel> servModel6 = CreateObject<ServiceModel > ();
    node6->AggregateObject(servModel6);
    Ptr<ClickBridge> click7 = CreateObject<ClickBridge > ();
    node7->AggregateObject(click7);
    click7->SetClickFile("/tmp//00000008.conf");
    Ptr<ServiceModel> servModel7 = CreateObject<ServiceModel > ();
    node7->AggregateObject(servModel7);

    Ptr<TopologyManager> tm = CreateObject<TopologyManager > ();
    tm->SetStartTime(Seconds(0.));
    tm->SetAttribute("Topology", StringValue("/tmp/topology.graphml"));
    node0->AddApplication(tm);

    Ptr<Subscriber> app3_0 = CreateObject<Subscriber > ();
    app3_0->SetStartTime(Seconds(2.34));
    app3_0->SetStopTime(Seconds(14.87));
    node3->AddApplication(app3_0);

    Ptr<Subscriber> app4_0 = CreateObject<Subscriber > ();
    app4_0->SetStartTime(Seconds(2.34));
    app4_0->SetStopTime(Seconds(14.87));
    node4->AddApplication(app4_0);

    Ptr<Publisher> app5_0 = CreateObject<Publisher > ();
    app5_0->SetStartTime(Seconds(2.34));
    app5_0->SetStopTime(Seconds(14.87));
    node5->AddApplication(app5_0);

    Ptr<Subscriber> app6_0 = CreateObject<Subscriber > ();
    app6_0->SetStartTime(Seconds(2.34));
    app6_0->SetStopTime(Seconds(14.87));
    node6->AddApplication(app6_0);

    Ptr<Subscriber> app7_0 = CreateObject<Subscriber > ();
    app7_0->SetStartTime(Seconds(2.34));
    app7_0->SetStopTime(Seconds(14.87));
    node7->AddApplication(app7_0);

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}