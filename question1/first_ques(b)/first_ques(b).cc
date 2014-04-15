#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include<string>


int prev_recv_packets;
int cur_recv_packets;

using namespace ns3;

void calculate_throughput(Ptr<UdpServer>);

int main (int argc, char *argv[])
{
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  
  cur_recv_packets=prev_recv_packets=0;
  
  NodeContainer nodes;
  nodes.Create (2);		//create nodes

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue (argv[1]));

  //installing devices
  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

   //setting stack for nodes
  InternetStackHelper stack;
  stack.Install (nodes);

   //assigning addresses
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  // First Instance of Client-Server Application
  UdpServerHelper echoServer_1 (9);

  ApplicationContainer serverApps_1 = echoServer_1.Install (nodes.Get (1));
  serverApps_1.Start (Seconds (1.0));
  serverApps_1.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient_1 (interfaces.GetAddress (1), 9);
  echoClient_1.SetAttribute ("MaxPackets", UintegerValue (10));
  echoClient_1.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient_1.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps_1 = echoClient_1.Install (nodes.Get (0));
  clientApps_1.Start (Seconds (2.0));
  clientApps_1.Stop (Seconds (10.0));

  // Second Instance of Client-Server Application
  UdpServerHelper echoServer_2 (10);

  ApplicationContainer serverApps_2 = echoServer_2.Install (nodes.Get (1));
  serverApps_2.Start (Seconds (1.0));
  serverApps_2.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient_2(interfaces.GetAddress (1), 10);
  echoClient_2.SetAttribute ("MaxPackets", UintegerValue (10));
  echoClient_2.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient_2.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps_2 = echoClient_2.Install (nodes.Get (0));
  clientApps_2.Start (Seconds (2.0));
  clientApps_2.Stop (Seconds (10.0));
 
  //storing the trace in ASCII format
  pointToPoint.EnableAsciiAll("q1.tr");

  pointToPoint.EnablePcapAll("q1");
  
  Simulator::Schedule(Seconds(1.0),&calculate_throughput,echoServer_1.GetServer());
  Simulator::Schedule(Seconds(1.0),&calculate_throughput,echoServer_2.GetServer());
  printf("Latency of the link set at:%s\n",argv[1]);

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

void calculate_throughput(Ptr<UdpServer> echoServer)
{
  //===============================================
  // Normal throughput calculation.
  // Throughput = Transfer Size/Transfer Time.
  //===============================================
        cur_recv_packets=echoServer->GetReceived();
        /* Get latency of the link. and get current time.*/
        printf("Measuring Throughput at time:%f ",ns3::Simulator::Now().GetSeconds());
        printf("Recieved Packets:%d\n",cur_recv_packets-prev_recv_packets);
        printf("Throughput:%f Bps(Bytes per Second)\n",((cur_recv_packets+0.0)*1024)/ns3::Simulator::Now().GetSeconds());
        prev_recv_packets=cur_recv_packets;
        if(ns3::Simulator::Now().GetSeconds()<10.000000000)
        Simulator::Schedule(Seconds(1.0),&calculate_throughput,echoServer); 
}
