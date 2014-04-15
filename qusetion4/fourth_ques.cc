#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include <iostream>
#include <fstream>

std::ofstream fp1, fp2;

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Lab3");

static double node2BytesRcv = 0;
static double node3BytesRcv = 0;

void PacketNode2 (std::string ,Ptr<const Packet> , const Address&);
void PacketNode3 (std::string ,Ptr<const Packet> , const Address&);

int main (int argc, char *argv[])
{
  fp1.open("fourth1.plot", std::ios::app);
  fp2.open("fourth2.plot", std::ios::app);

  std::string tcpType = "NewReno";
  uint16_t port = 50000;
 
 // setting socket type
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
 // setting tcp type
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue(TypeId::LookupByName ("ns3::Tcp" + tcpType)));
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", UintegerValue(uint32_t(10)));
  
  NS_LOG_INFO ("Creating Topology");
  NodeContainer nodes;
  nodes.Create (4);   // creating nodes
  
  PointToPointHelper link;
  link.SetChannelAttribute("Delay", TimeValue(Time("10ms")));

  // creating n0-n1 link
  NodeContainer n0n1 (nodes.Get (0), nodes.Get (1));
  link.SetDeviceAttribute("DataRate", DataRateValue(DataRate("10Mbps")));
  NetDeviceContainer d0d1 = link.Install (n0n1);
  
  //creating n2-n0 link
  NodeContainer n2n0 (nodes.Get (2), nodes.Get (0));
  link.SetDeviceAttribute("DataRate", DataRateValue(DataRate("1.5Mbps")));
  NetDeviceContainer d2d0 = link.Install(n2n0);
  
  //creating n3-n0 link
  NodeContainer n3n0 (nodes.Get (3), nodes.Get (0));
  const std::string delay(argv[1]);
  link.SetChannelAttribute("Delay", TimeValue(Time(delay)));
  NetDeviceContainer d3d0 = link.Install(n3n0);


  //setting stack for nodes
  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  
  //assigning address to n0-n1 interface
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interface01 = address.Assign (d0d1);

  //assigning address to n2-n0 interface
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer interface20 = address.Assign (d2d0);

  //assigning address to n3-n0 interface
  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer interface30 = address.Assign (d3d0);
  
  ApplicationContainer apps;
  
  // Traffic Source
  OnOffHelper source("ns3::TcpSocketFactory", InetSocketAddress(interface01.GetAddress (1), port));
  source.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=5]"));
  source.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  source.SetAttribute ("DataRate", DataRateValue (DataRate ("1.5Mbps")));
  source.SetAttribute ("PacketSize", UintegerValue (2000));

  apps.Add (source.Install (nodes.Get (2)));
  
  source.SetAttribute ("Remote", AddressValue(InetSocketAddress(interface01.GetAddress (1), port + 1)));
  apps.Add (source.Install (nodes.Get (3)));
  
  // Traffic Sink
  PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress(interface01.GetAddress (1), port));
  apps.Add(sink.Install (nodes.Get (1)));
  
  sink.SetAttribute("Local", AddressValue(InetSocketAddress(interface01.GetAddress(1), port + 1)));
  apps.Add (sink.Install (nodes.Get (1)));
 
  // enabling global routing
  NS_LOG_INFO ("Enable static global routing.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
  Ptr<PointToPointRemoteChannel> linkChannel ((PointToPointRemoteChannel*) &(*(nodes.Get(3)->GetDevice (0)->GetChannel ())));
  
  std::string context = "/NodeList/1/ApplicationList/0/$ns3::PacketSink/Rx";
  Config::Connect (context, MakeCallback(&PacketNode2));
  
  context = "/NodeList/1/ApplicationList/1/$ns3::PacketSink/Rx";
  Config::Connect (context, MakeCallback(&PacketNode3));
  
  AsciiTraceHelper ascii;
  link.EnableAsciiAll (ascii.CreateFileStream ("lab3-rtt.tr"));

  apps.Start(Seconds (0.0));
  apps.Stop(Seconds (5.0));

  Simulator::Stop(Seconds(5.0));
  Simulator::Run();
  
  fp1 << (node2BytesRcv * 8 / 1000000) / (5.0) << " " << delay << std::endl;
  fp2 << (node3BytesRcv * 8 / 1000000) / (5.0) << " " << delay << std::endl;
  
  std::cout << ". Delay : " << delay  << "\n";
  std::cout << ". Throughput from Node 2: " << (node2BytesRcv * 8 / 1000000) / (5.0) << " Mbps" << std::endl;
  std::cout << ". Throughput from Node 3: " << (node3BytesRcv * 8 / 1000000) / (5.0) << " Mbps" << std::endl;
		
  Simulator::Destroy();
  fp1.close();
  fp2.close(); 
  return 0;
}

//processing the packet received from node 2
void PacketNode2 (std::string context, Ptr<const Packet> p, const Address& addr)
{
  NS_LOG_INFO (context << " Packet Received from Node 2 at " << Simulator::Now ().GetSeconds());
  node2BytesRcv += p->GetSize ();
}

//processing the packet received from node 3
void PacketNode3 (std::string context, Ptr<const Packet> p, const Address& addr)
{
  NS_LOG_INFO (context << " Packet Received from Node 3 at " << Simulator::Now ().GetSeconds() << "from " << InetSocketAddress::ConvertFrom(addr).GetIpv4 ());
  node3BytesRcv += p->GetSize ();
}
