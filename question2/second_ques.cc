#include <iostream>
#include<fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/service-flow.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("DynamicRoutingProtocol");
FlowMonitorHelper helper;
Ptr<FlowMonitor> monitor;

void packet_loss(double);
 std::ofstream fout;
 
int main (int argc, char *argv[])
{
  Config::SetDefault("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents",BooleanValue(true));
  LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  bool enableFlowMonitor = true;
  
  fout.open("loss.txt");
  
  CommandLine cmd;
  cmd.AddValue ("EnableMonitor", "Enable Flow Monitor", enableFlowMonitor);
  cmd.Parse(argc,argv);
  
  NodeContainer p2pNodes;
  p2pNodes.Create (5);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevice01, p2pDevice03, p2pDevice12, p2pDevice34, p2pDevice24;

  //creatiing n0-n1 link
  p2pDevice01 = pointToPoint.Install (p2pNodes.Get (0),p2pNodes.Get (1));

  //creating n0-n3 link
  p2pDevice03 = pointToPoint.Install (p2pNodes.Get (0),p2pNodes.Get (3));

  //creating n1-n2 link 
  p2pDevice12 = pointToPoint.Install (p2pNodes.Get (1),p2pNodes.Get (2));

  //creating n4-n2 link
  p2pDevice24 = pointToPoint.Install (p2pNodes.Get (2),p2pNodes.Get (4));

  //creating n3-n4 link
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("0.5Mbps"));
  p2pDevice34 = pointToPoint.Install (p2pNodes.Get (3),p2pNodes.Get (4));

  InternetStackHelper stack;
  stack.Install (p2pNodes);

  Ipv4AddressHelper address;
  Ipv4InterfaceContainer p2pInterface01, p2pInterface03, p2pInterface12, p2pInterface34, p2pInterface24;

  //assigning address to n0-n1 interface
  address.SetBase ("10.1.1.0", "255.255.255.0");
  p2pInterface03 = address.Assign (p2pDevice03);

  //assigning address to n0-n3 interface
  address.SetBase ("10.1.2.0", "255.255.255.0");
  p2pInterface01 = address.Assign (p2pDevice01);
  
  //assigning address to n1-n2 interface
  address.SetBase ("10.1.3.0", "255.255.255.0");
  p2pInterface12 = address.Assign (p2pDevice12);

  //assigning address to n3-n4 interface
  address.SetBase ("10.1.4.0", "255.255.255.0");
  p2pInterface24 = address.Assign (p2pDevice24);

  //assigning address to n4-n2 interface
  address.SetBase ("10.1.5.0", "255.255.255.0");
  p2pInterface34 = address.Assign (p2pDevice34);

  NS_LOG_INFO ("Enable static global routing.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  OnOffHelper source1("ns3::UdpSocketFactory",InetSocketAddress(p2pInterface12.GetAddress(1),9));
  source1.SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  source1.SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  source1.SetAttribute("DataRate", StringValue ("900kbps"));
  source1.SetAttribute("PacketSize", UintegerValue(50));

  ApplicationContainer sourceApps1 = source1.Install(p2pNodes.Get(1));
  sourceApps1.Start(Seconds(1.));
  sourceApps1.Stop(Seconds(3.5));

  OnOffHelper source2("ns3::UdpSocketFactory",InetSocketAddress(p2pInterface12.GetAddress(1),9));
  source2.SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  source2.SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  source2.SetAttribute("DataRate", StringValue ("300kbps"));
  source2.SetAttribute("PacketSize", UintegerValue(50));

  ApplicationContainer sourceApps2 = source2.Install(p2pNodes.Get(1));
  sourceApps2.Start(Seconds(1.5));
  sourceApps2.Stop(Seconds(3.));

  int16_t sinkPort = 9;
  PacketSinkHelper sink1("ns3::UdpSocketFactory",InetSocketAddress(Ipv4Address::GetAny(),sinkPort));
  ApplicationContainer sinkApps1 = sink1.Install(p2pNodes.Get(0));
  sinkApps1.Start(Seconds(1.));
  sinkApps1.Stop(Seconds(3.5));

  PacketSinkHelper sink2("ns3::UdpSocketFactory",InetSocketAddress(Ipv4Address::GetAny(),sinkPort));
  ApplicationContainer sinkApps2 = sink2.Install(p2pNodes.Get(0));
  sinkApps2.Start(Seconds(1.5));
  sinkApps2.Stop(Seconds(3.));

  Ptr<Node> node = p2pNodes.Get(1);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
  uint32_t interface = 1;//ipv4->GetInterfaceForPrefix(Ipv4Address("10.1.1.0") ,Ipv4Mask("255.255.255.0"));
  Simulator::Schedule(Seconds (2.0), &Ipv4::SetDown, ipv4,interface);
  Simulator::Schedule(Seconds (2.7), &Ipv4::SetUp, ipv4, interface);
  
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("q2.tr");
  pointToPoint.EnableAsciiAll (stream);

  stack.EnableAsciiIpv4All (stream);
  pointToPoint.EnablePcapAll("DynamicRoutingProtocol");

  
  if (enableFlowMonitor)
    {
      monitor = helper.InstallAll();
      //monitor = flowmon.Install (nodes.Get (0));
      monitor->Start (Seconds (0.5));
    }

  for(double k=1.00;k<=3.5;k = k + 0.05)
  {
    Simulator::Schedule (Seconds(k), &packet_loss, k);
  }
  
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (8.0));
  Simulator::Run ();

  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
  fout.close();
  return 0;
}

void packet_loss(double k)
{
  double total=0;
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (helper.GetClassifier ());
  map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  
  map<FlowId, FlowMonitor::FlowStats>::const_iterator i;
  
  double j=0,m=0;
  for ( i = stats.begin (); i != stats.end (); ++i)
  {		
                Ipv4FlowClassifier::FiveTuple l = classifier->FindFlow (i->first);
                if(l.destinationAddress == "10.1.3.2"){
                  j += i->second.txPackets;
                  m += i->second.lostPackets;
                }
  }
  total = m/j ;
  fout<< k <<" "<< total << std::endl;
}
