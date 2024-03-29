#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include<stdio.h>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/service-flow.h"
#include "ns3/flow-monitor-module.h"

#include "ns3/tcp-l4-protocol.h"

#include "/home/gomchik/ns-allinone-3.19/ns-3.19/src/core/model/nstime.h"
#include "/home/gomchik/ns-allinone-3.19/ns-3.19/src/internet/model/tcp-newreno.h"

#include "ns3/gnuplot.h"
#include "ns3/global-route-manager.h"


using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("Lab3");
int pkt_count=0,tot_length=0,pkt_size[10],tot_pkt_size,queue_size;
float t_ini[10],t_final,time_diff;
FILE *fp[6];

void EnqueuePacket(std::string context, Ptr<const Packet> p)
{
  queue_size++;   
  fprintf(fp[5],"%f \t E \t %d \n",Simulator::Now ().GetSeconds(),queue_size);       
}

void DequeuePacket(std::string context, Ptr<const Packet> p)
{
  queue_size--;
  fprintf(fp[5],"%f \t D \t %d \n",Simulator::Now ().GetSeconds(),queue_size);       
}

void DropPacket(std::string context, Ptr<const Packet> p)
{   
  fprintf(fp[5],"%f \t X \t %d \n",Simulator::Now ().GetSeconds(),queue_size);       
}

void ReceivePacket (std::string context, Ptr<const Packet> p, const Address& addr)
{
        int i,port;        
        port = InetSocketAddress::ConvertFrom(addr).GetPort();        
        pkt_count++;
        if(pkt_count<10)
        {
        
                t_ini[pkt_count]= Simulator::Now ().GetSeconds();
                pkt_size[pkt_count]= p->GetSize();        
        }
        else
        {
                                
                t_final = Simulator::Now ().GetSeconds();       
                time_diff = t_final - t_ini[pkt_count%10];                               
                tot_pkt_size=0;                
                for(i=0;i<10;i++)
                        tot_pkt_size+=pkt_size[i];                
                tot_length=tot_pkt_size/time_diff;                         
                
                if(port==49153)
                {
                  fprintf(fp[0],"%f \t %d \n",t_final,tot_length);       
                }
                else if(port==49154)
                {
                  fprintf(fp[1],"%f \t %d \n",t_final,tot_length);          
                 
                } 
                 else if(port==49155)
                {
                  fprintf(fp[2],"%f \t %d \n",t_final,tot_length);          
                 
                }   
                else if(port==49156)
                {
                  fprintf(fp[3],"%f \t %d \n",t_final,tot_length);          
                 
                }   
                else if(port==49157)
                {
                  fprintf(fp[4],"%f \t %d \n",t_final,tot_length);         
                 
                }   
                                        
                t_ini[pkt_count%10]=t_final;
                pkt_size[pkt_count%10]=p->GetSize();                
        }
}

int main (int argc, char *argv[])
{
  
  // disable fragmentation
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // Set the size of the sending queue
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", UintegerValue(uint32_t(10)));
  LogComponentEnable("Lab3", LOG_LEVEL_INFO);
  
  uint32_t nFlows = 5;
  std::string tcpType = "NewReno";
  uint16_t port = 50000;

   fp[0]=fopen("port1.dat","w");
   fp[1]=fopen("port2.dat","w");
   fp[2]=fopen("port3.dat","w");          
   fp[3]=fopen("port4.dat","w");
   fp[4]=fopen("port5.dat","w");
   fp[5]=fopen("queuesize.dat","w"); 
   int i;
   for(i=0;i<10;i++)
   {
        t_ini[i]=0;
        pkt_size[i]=0;
   }
  // Set default Socket type to one of the Tcp Sockets
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue(TypeId::LookupByName ("ns3::Tcp" + tcpType)));

  NS_LOG_INFO ("# Creating Topology");

  NodeContainer nodes;
  nodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", DataRateValue(DataRate("10Mbps")));
  pointToPoint.SetChannelAttribute("Delay", TimeValue(Time("10ms")));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  ApplicationContainer serverApp[nFlows];
  ApplicationContainer sinkApp[nFlows];
  
  for(unsigned int i = 0;i < nFlows; i++){
    PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress(interfaces.GetAddress (1), port + i));
    sinkApp[i] = packetSinkHelper.Install (nodes.Get (1));
    sinkApp[i].Start(Seconds (0.0));
    sinkApp[i].Stop(Seconds (60.0));
  }
  
  for(unsigned int i = 0; i < nFlows; i++){
    OnOffHelper server("ns3::TcpSocketFactory", InetSocketAddress(interfaces.GetAddress (1), port + i));
    server.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=60]"));
    server.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    server.SetAttribute ("DataRate", DataRateValue (DataRate ("1.5Mbps")));
    server.SetAttribute ("PacketSize", UintegerValue (2000));
    
    serverApp[i] = server.Install (nodes.Get (0));
    serverApp[i].Start(Seconds (i * 5));
    if(i != 0)
    serverApp[i].Stop (Seconds (50.0 - (i * 5)));
    else 
    serverApp[i].Stop (Seconds (50.1));
  }

  NS_LOG_INFO ("Enable static global routing.");
  // Turn on global static routing so we can actually be routed across the star.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
  AsciiTraceHelper ascii;
  pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("lab3-nflow.tr"));
  
  std::string context = "/NodeList/0/DeviceList/0/$ns3::PointToPointNetDevice/TxQueue/";
  
  Config::Connect (context + "Enqueue", MakeCallback (&EnqueuePacket));
  Config::Connect (context + "Dequeue", MakeCallback (&DequeuePacket));
  Config::Connect (context + "Drop", MakeCallback (&DropPacket));
  
  context = "/NodeList/1/ApplicationList/*/$ns3::PacketSink/Rx";
  Config::Connect (context, MakeCallback(&ReceivePacket));
  Simulator::Run ();
  Simulator::Destroy ();
  fclose(fp[0]);
  fclose(fp[1]);
  fclose(fp[2]);
  fclose(fp[3]);
  fclose(fp[4]);
  fclose(fp[5]);
  return 0;
}
