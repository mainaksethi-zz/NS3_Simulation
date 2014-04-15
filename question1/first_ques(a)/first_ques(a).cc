#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"


int prev_recv_packets;
int cur_recv_packets;
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("UDP_point-to-point");

void calculate_throughput(Ptr<UdpServer>);

int main (int argc, char *argv[])
{

  /* Enabling debug logging at the INFO level for echo clients and servers */
  LogComponentEnable("UDP_point-to-point", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  cur_recv_packets=prev_recv_packets=0;
  /* The NodeContainer topology helper provides a convenient way to create, 
  manage and access any Node objects that we create in order to run a 
  simulation.*/ 

  NodeContainer nodes; 
  nodes.Create (2);   //create nodes

  PointToPointHelper helper;
  //Setting channel attributes 
  helper.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  helper.SetChannelAttribute ("Delay", StringValue(argv[1]));

  // Installing devices
  NetDeviceContainer devices;
  devices = helper.Install (nodes);
  
  //setting stack for nodes
  InternetStackHelper stack;
  stack.Install (nodes);

   //assigning addresses
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  // creating server instance
  NS_LOG_INFO("Create server applications.");
  UdpServerHelper echoServer (9); //(9) is just port which is attached
  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));
  UdpEchoServerHelper echoServer2 (10);
  ApplicationContainer serverApps2 = echoServer2.Install (nodes.Get (1));
  serverApps2.Start (Seconds (1.0));
  serverApps2.Stop (Seconds (10.0));

  // creating client instance
  NS_LOG_INFO("Create client application.");
  UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (5));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
  UdpEchoClientHelper echoClient2 (interfaces.GetAddress (1), 10);
  echoClient2.SetAttribute ("MaxPackets", UintegerValue (5));
  echoClient2.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient2.SetAttribute ("PacketSize", UintegerValue (1024));
  ApplicationContainer clientApps2 = echoClient2.Install (nodes.Get (0));
  clientApps2.Start (Seconds (2.0));
  clientApps2.Stop (Seconds (10.0));

  NS_LOG_INFO("Enable capturing of trace.");
  AsciiTraceHelper ascii;
  helper.EnableAsciiAll (ascii.CreateFileStream ("first_modified.tr"));
  helper.EnablePcapAll ("first_modified");


  Simulator::Schedule(Seconds(1.0),&calculate_throughput,echoServer.GetServer());
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
