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
#include <string>
#include <sstream>

using namespace ns3;
using namespace std;


typedef struct a {
	NodeContainer nodes;
	struct a *prox;
} Fila;

Fila *clientes = NULL;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

void criaCliente(){
	Fila *aux;
	Fila *temp;
	
	aux = (Fila *) malloc(sizeof(Fila));
	
	
	InternetStackHelper stack;
	
	aux->nodes.Create(5);
	aux->prox = NULL;
	
	stack.Install(aux->nodes);
	
	if (clientes == NULL){
		clientes = aux;
	} else {
		temp = clientes;
		
		while (temp->prox != NULL) temp = temp->prox;
		
		temp->prox = aux;
	}
}

NodeContainer getNodes(int n){
	Fila *temp = clientes;
	
	for (i=0;i<n;i++){
		temp = temp->prox;
	}
	
	return temp->nodes;
	
}

int main (int argc, char *argv[]){
	bool tracing = true;
	bool verbose = true;
	uint32_t nCsma = 3;
    uint32_t nWifi = 10;
	
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
	
	NodeContainer apNodeContainer;
	apNodeContainer.Create(5);
	
	NodeContainer server;
	server.Create(1);
	
	
	//Seta as configuraçes de comunicaçao
	PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
    
    NetDeviceContainer apConnection;
   
   	for (int i=0; i<5;i++){
   		apConnection.Add(pointToPoint.Install(apNodeContainer.Get(i), server.Get(0)));
   	}
    
	//Nesse momento, temos todos os Ap's linkados ao server, temos apenas que settar as interfaces
	
	for (int i=0; i<5 ; i++){
		criaCliente();
	}
    
	//Cria pilha de internet
	InternetStackHelper pilha;
    pilha.Install (apNodeContainer);
    pilha.Install (server);
    	
    
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
	phy.SetChannel (channel.Create());
	
	WifiHelper wifi;
    wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
	WifiMacHelper mac;
	
	
	Ssid ssid = Ssid ("Rede1");
	char nome[] = "Rede1";
	
	NetDeviceContainer clientWifiDevices;
	NetDeviceContainer apDevices;
	
	for (int i=0;i<5;i++){
		nome[4] = (char)(49+i)
		ssid = Ssid(nome)
		
		mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue (false));
		staDevices.Add(wifi.Install (phy, mac, getNodes(i)));   
		
		mac.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssid));
		apDevices.Add(wifi.Install (phy, mac, apNodeContainer.Get(i)));
	}
	
	//Cria 5 interfaces que conectam no servidor
    Ipv4InterfaceContainer interface;
   	Ipv4AddressHelper endereco;

	endereco.SetBase ("10.1.6.0", "255.255.255.0");//Interface P2P
	interface.Add(endereco.Assign(apConnection))
	
	char enderecoIp[] = "10.1.1.0"
	
	for (int i=0;i<5;i++){
		enderecoIp[5] = (char)(49+i)
		
		endereco.SetBase (enderecoIP, "255.255.255.0");//Interface Wifi
		endereco.Assign (getNodes(i));
		endereco.Assign (apNodeContainer(i));
	}
    

	
	
	/*Nao faz sentido os clientes que utilizam Wifi ficarem parados
	portanto, sera utilizado o mobilityHelper para fazerem os nos se moverem, enquanto
	o ponto de acesso fica parado*/
	
	
	/*
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
	*/
	
	
	//Cria o servidor de eco UDP (Espera os pacotes UDP, e retorna eles para quem os enviou)
	UdpEchoServerHelper echoServer (9);

    ApplicationContainer serverApps = echoServer.Install (server.Get (0));//Setta o servidor de aplicação para ser o server
    serverApps.Start (Seconds (1.0));
    serverApps.Stop (Seconds (10.0));
	
	//Faz a mesma coisa com a interface Wifi, porem, ao inves de criar outro servidor, apenas faz o no apontar para o servidor ja criado na interface CSMA
	/*
	UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (3), 9);//Pega o endereço do no 3 CSMA, que e o servidor de eco UDP
    echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));*/

    ApplicationContainer clientApps = echoClient.Install (cliente->nodes.Get(0));//Seta o servidor no no 2 wifi
    clientApps.Start (Seconds (2.0));
    clientApps.Stop (Seconds (10.0));
	
	//Habilita o roteamento intra-net
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	
	Simulator::Stop (Seconds (10.0)); //Para a simulaçao
	
	//Faz o rastreamento das redes para ver o que esta acontecendo
	if (verbose){
		pointToPoint.EnablePcapAll ("P2PNodes");
		phy.EnablePcap ("ApNode", apDevices.Get (0));
		csma.EnablePcap ("CSMANodes", servidor.Get (0), true);
	}

	Simulator::Run ();
    Simulator::Destroy ();
    return (0);
}
