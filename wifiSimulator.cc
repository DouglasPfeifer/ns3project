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
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include <bits/stdc++.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int main (int argc, char *argv[]){
	bool tracing = true;
	bool verbose = true;
	uint32_t nCsma = 3;
    uint32_t nWifi = 3;
	
	CommandLine cmd;
    cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
    cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
    cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
    cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

    cmd.Parse (argc,argv);
	
	if (nWifi > 18){
		std::cout << "O numero de dispositivos wifi deve ser menor que 18, caso contrario, vai exceder a boundingBox do grid" << std::endl;
		return 1;
    }

	if (verbose){
		LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
		LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }
	
	
	
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
	WifiHelper wifi;
    wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
	WifiMacHelper mac;
	
	Ssid ssid = Ssid ("RC2-Example"); //Nome da rede
    mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue (false));
	
	//Cria os dispositivos Wifi
	NetDeviceContainer staDevices;
    staDevices = wifi.Install (phy, mac, wifiStaNodes);
	
	//Configura o Ponto de acesso
	mac.SetType ("ns3::NqapWifiMac", "Ssid", SsidValue (ssid),"BeaconGeneration", BooleanValue (true),"BeaconInterval", TimeValue (Seconds (2.5)));
	//--O que e beaconGeneration e BeaconInterval  
	  
	//Uma vez configurado, basta instalar o Ponto de acesso
	NetDeviceContainer apDevices;
    apDevices = wifi.Install (phy, mac, wifiApNode);
	
	
	/*Nao faz sentido os clientes que utilizam Wifi ficarem parados
	portanto, sera utilizado o mobilityHelper para fazerem os nos se moverem, enquanto
	o ponto de acesso fica parado*/
	
	MobilityHelper mobility;

    mobility.SetPositionAllocator ("ns3::GridPositionAllocator", "MinX", DoubleValue (0.0),"MinY", DoubleValue (0.0),"DeltaX",
                                                DoubleValue (5.0),"DeltaY", DoubleValue (10.0), "GridWidth", UintegerValue (3),"LayoutType", StringValue ("RowFirst"));
	//--Pesquisar como exatamente funciona o GridPositionAllocator
	mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel","Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
	//--O algoritmo RandomWalk2dMobilityModel faz os hosts wifi se moverem de forma aleatoria com velocidades diferentes
	
	mobility.Install (wifiStaNodes); 
	
	//Seta o ponto de acesso para ficar fixo (ConstantPositionMobilityModel)
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (wifiApNode);
	
	//Cria a pilha de protocolos
	
	InternetStackHelper stack;
    stack.Install (csmaNodes);
    stack.Install (wifiApNode);
    stack.Install (wifiStaNodes);
	
	//Seta IP's para as interfaces
	
	Ipv4AddressHelper address;

    address.SetBase ("10.1.1.0", "255.255.255.0");//Interface P2P
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign (p2pDevices);

    address.SetBase ("10.1.2.0", "255.255.255.0");//Interface CSMA
    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = address.Assign (csmaDevices);

    address.SetBase ("10.1.3.0", "255.255.255.0");//Interface Wifi
    address.Assign (staDevices);
    address.Assign (apDevices);
	
	
	//Cria o servidor de eco UDP (Espera os pacotes UDP, e retorna eles para quem os enviou)
	UdpEchoServerHelper echoServer (9);

    ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (3));//Faz para a interface CSMA (o servidor fica sendo o no 3)
    serverApps.Start (Seconds (1.0));
    serverApps.Stop (Seconds (10.0));
	
	//Faz a mesma coisa com a interface Wifi, porem, ao inves de criar outro servidor, apenas faz o no apontar para o servidor ja criado na interface CSMA
	
	UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (3), 9);//Pega o endereço do no 3 CSMA, que e o servidor de eco UDP
    echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

    ApplicationContainer clientApps = echoClient.Install (wifiStaNodes.Get (2));//Seta o servidor no no 2 wifi
    clientApps.Start (Seconds (2.0));
    clientApps.Stop (Seconds (10.0));
	
	//Habilita o roteamento intra-net
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	
	Simulator::Stop (Seconds (10.0)); //Para a simulaçao
	
	//Faz o rastreamento das redes para ver o que esta acontecendo
	pointToPoint.EnablePcapAll ("third");
    phy.EnablePcap ("third", apDevices.Get (0));
	csma.EnablePcap ("third", csmaDevices.Get (0), true);
	
	Simulator::Run ();
    Simulator::Destroy ();
    return (0);
}