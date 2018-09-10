 // Topologia da rede
 //
 // Wifi 10.1.3.0
 //                    AP
 //   *    *     *     *
 //   |     |     |      |           10.1.1.0
 // n5   n6   n7   n0  - - - - - - - - - - - - - -  n1   n2   n3   n4
 //                             ponto-a-ponto       |     |      |      |
 //                                                       ===========
 //                                                          LAN 10.1.2.0s

#include "ns3/core-module.h"
#include "ns3/simulator-module.h"
#include "ns3/node-module.h"
#include "ns3/helper-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include <bits/stdc++.h>

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int main (int argc, char *argv[]){
	uint32_t nCsma = 3;
    uint32_t nWifi = 3;
	
	//Cria dois nos
	NodeContainer p2pNodes;
	p2pNodes.Create (2);
	
	//Seta as configuraçes de comunicaçao
	PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

	//Faz a conexo ponto a ponto
    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install (p2pNodes);
	
	//Seta o no 1 para ser conexao csma (para rede com fio)
	NodeContainer csmaNodes;
	csmaNodes.Add (p2pNodes.Get (1));
	csmaNodes.Create (nCsma);
	
	CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute ("Delay", TimeValue(NanoSeconds (6560)));

    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install (csmaNodes);
	
	//Cria os nos que farao parte da rede wifi
	NodeContainer wifiStaNodes;
    wifiStaNodes.Create (nWifi);
	
	//Seta o no 0 para ser o ponto de acesso
    NodeContainer wifiApNode = p2pNodes.Get(0);
	
	//Cria a conexao wifi
	YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
	phy.SetChannel (channel.Create());
	
	//Configura o MAC
	WifiHelper wifi = WifiHelper::Default ();
    wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

    NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();
	
	Ssid ssid = Ssid ("RC2-Example"); //Nome da rede
    mac.SetType ("ns3::NqstaWifiMac", "Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue (false));
}