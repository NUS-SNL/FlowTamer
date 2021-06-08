#if !defined(WIN32) && !defined(WINx64)
#include <netinet/in.h> // this is for using ntohs() and htons() on non-Windows OS's
#endif
#include <unistd.h>
#include <EthLayer.h>
#include <IPv4Layer.h>
#include <TcpLayer.h>
#include <inttypes.h>
// #include "PlatformSpecificUtils.h"
#include <unordered_map>
#include <chrono>
#include <iostream>

#include "pcpp/WorkerThread.h"
#include "utils/types.hpp"
#include "bfrt/bfrt.hpp"
#include "utils/utils.hpp"

struct wsKey{
	uint32_t srcIP;
	uint16_t srcPort;
	bool operator==(const wsKey& w1) const
	{
		return (srcIP == w1.srcIP && srcPort == w1.srcPort);		
	}
};
class wsMapHashFn {
public:
    size_t operator()(const wsKey& w1) const
    {
        return w1.srcIP ^ w1.srcPort;
    }
};
struct rttKey{
	uint32_t srcIP;
	uint32_t dstIP;
	uint16_t srcPort;
	uint16_t dstPort;
	bool operator==(const rttKey& r1) const
	{
		return (srcIP == r1.srcIP && dstIP == r1.dstIP && srcPort == r1.srcPort && dstPort == r1.dstPort);		
	}
};
class rttMapHashFn {
public:
    size_t operator()(const rttKey& r1) const
    {
        return r1.srcIP ^ r1.dstIP ^ r1.srcPort ^ r1.dstPort;
    }
};

std::unordered_map<wsKey, ws_t, wsMapHashFn> WS_MAP;
std::unordered_map<rttKey, rtt_t, rttMapHashFn> RTT_MAP;

// struct FlowKey{
// 	uint32_t srcIP;
// 	uint32_t dstIP;
// 	uint8_t protoType;
// 	uint16_t srcPort;
// 	uint16_t dstPort;

// 	bool operator==(const FlowKey& f1) const
// 	{
// 		return (srcIP == f1.srcIP && dstIP == f1.dstIP && protoType == f1.protoType && srcPort == f1.srcPort && dstPort == f1.dstPort);		
// 	}
// };
// struct FlowValue{
// 	std::string connState;
// 	uint32_t expSeqOrAckNo;
// 	uint8_t senderWS;
// 	uint8_t receiverWS;
// 	uint16_t senderRwnd;
// 	uint16_t receiverRwnd;
// };


// class MyHashFunction {
// public:
//     size_t operator()(const FlowKey& f1) const
//     {
//         return f1.srcIP ^ f1.dstIP ^ f1.srcPort ^ f1.dstPort ^ f1.protoType;
//     }
// };
// bool operator<(const FlowKey& f1, const FlowKey& f2) {
//     if(f1.srcIP < f2.srcIP){ return true; }
// 	else if(f1.srcIP > f2.srcIP){ return false; }
// 	else if(f1.dstIP < f2.dstIP){ return true; }
// 	else if(f1.dstIP > f2.dstIP){ return false; }
// 	else if(f1.protoType < f2.protoType){ return true; }
// 	else if(f1.protoType > f2.protoType){ return false; }
// 	else if(f1.srcPort < f2.srcPort){ return true; }
// 	else if(f1.srcPort > f2.srcPort){ return false; }
// 	else if(f1.dstPort < f2.dstPort){ return true; }
// 	else if(f1.dstPort > f2.dstPort){ return false; }
// 	else{ return true; }
// }
// std::unordered_map<FlowKey, FlowValue, MyHashFunction> FLOW_MAP;
// uint16_t ACTIVE_FLOWS = 0;

void print_ws_map(){
	for(auto i: WS_MAP){
		pcpp::IPv4Address srcIP(i.first.srcIP);
		
		printf("Key: \nsrcIP: %s\nsrcPort: %d\n", srcIP.toString().c_str(), ntohs(i.first.srcPort));
		printf("Value: \nws: %d\n\n", i.second);
	}
}
void print_rtt_map(){

	for(auto i: RTT_MAP){
		
		pcpp::IPv4Address srcIP(i.first.srcIP);
		pcpp::IPv4Address dstIP(i.first.dstIP);

		printf("Key: \nsrcIP: %s\ndstIP: %s\nsrcPort: %u\ndstPort: %u\n", srcIP.toString().c_str(), dstIP.toString().c_str(), ntohs(i.first.srcPort), ntohs(i.first.dstPort));
		printf("Value: \ntstmp: %lu\n\n", i.second);
	}
}

SenderWorkerThread::SenderWorkerThread(pcpp::DpdkDevice* txDevice) :
	m_TxDevice(txDevice), m_Stop(true), m_CoreId(MAX_NUM_OF_CORES+1)
{
}
bool SenderWorkerThread::run(uint32_t coreId)
{
	// Register coreId for this worker
	m_CoreId = coreId;
	m_Stop = false;

	int PKT_COUNT_PER_BATCH = 10;
	int BATCH_COUNT = 10;
	pcpp::Packet* packetArr[BATCH_COUNT][PKT_COUNT_PER_BATCH];
	
	for(int i=0;i<BATCH_COUNT;i++){
		for(int j=0; j<PKT_COUNT_PER_BATCH; j++){
			//printf("%d\n", i*PKT_COUNT_PER_BATCH + j + 1);
			// create a new Ethernet layer               // mac address interpreted as 0x11:0x22:0x33:0x44....
			pcpp::EthLayer newEthernetLayer(pcpp::MacAddress("01:01:01:01:01:01"), pcpp::MacAddress("22:22:22:22:22:22"));
			// make sure you write zero even though it is at the front
			// create a new IPv4 layer
			pcpp::IPv4Layer newIPLayer(pcpp::IPv4Address(std::string("1.1.1.1")), pcpp::IPv4Address(std::string("2.2.2.2")));

			// create a new UDP layer
			pcpp::TcpLayer newTcpLayer(htons(1), htons(2));
			newTcpLayer.getTcpHeader()->synFlag = 1;
			
			newTcpLayer.addTcpOption(pcpp::TcpOptionBuilder(pcpp::PCPP_TCPOPT_WINDOW, (uint8_t)8));
			// create a packet with initial capacity of 100 bytes (will grow automatically if needed)
			pcpp::Packet* newPacket = new pcpp::Packet(64);

			// add all the layers we created
			newPacket->addLayer(&newEthernetLayer);
			newPacket->addLayer(&newIPLayer);
			newPacket->addLayer(&newTcpLayer);

			// compute all calculated fields
			newPacket->computeCalculateFields();
			packetArr[i][j] = newPacket;
		}
		m_TxDevice->sendPackets(packetArr[i], PKT_COUNT_PER_BATCH, 0);
		usleep(1000);
	}
	printf("press ctrl +c to exit app\n");
	return true;
}

void SenderWorkerThread::stop()
{
	m_Stop = true;
}

uint32_t SenderWorkerThread::getCoreId() const
{
	return m_CoreId;
}

ReceiverWorkerThread::ReceiverWorkerThread(pcpp::DpdkDevice* rxDevice) :
	m_RxDevice(rxDevice), m_Stop(true), m_CoreId(MAX_NUM_OF_CORES+1)
{
}

bool ReceiverWorkerThread::run(uint32_t coreId)
{
	// Register coreId for this worker
	m_CoreId = coreId;
	m_Stop = false;

	std::chrono::high_resolution_clock::time_point start_time, end_time, bfrt_end_time;
	long long duration_usecs, bfrt_duration_usecs;

	// auto s = std::chrono::high_resolution_clock::now();

	Bfruntime& bfrt = Bfruntime::getInstance();
	rtt_ws_entry_pair_info_t rtt_ws_info;
	ws_t srcWS, dstWS;
	bf_status_t status;

	// initialize a packet array of size 64
	pcpp::Packet* packetArr[64] = {};
	// endless loop, until asking the thread to stop
	while (!m_Stop)
	{
		// receive packets from RX device
		uint16_t numOfPackets = m_RxDevice->receivePackets(packetArr, 64, 0);

		if (numOfPackets > 0)
		{
			// printf("Received number of pkts: %d\n", numOfPackets);

			for(int i=0; i<numOfPackets; i++){
				
				start_time = std::chrono::high_resolution_clock::now();

				if(packetArr[i]->isPacketOfType(pcpp::TCP)){
					pcpp::TcpLayer* tcpLayer = packetArr[i]->getLayerOfType<pcpp::TcpLayer>();
					pcpp::IPv4Layer* ipLayer = packetArr[i]->getLayerOfType<pcpp::IPv4Layer>();
					pcpp::EthLayer* ethLayer = packetArr[i]->getLayerOfType<pcpp::EthLayer>();

					pcpp::tcphdr* tcpHeader = tcpLayer->getTcpHeader();
					pcpp::iphdr* ipHeader = ipLayer->getIPv4Header();

					uint8_t srcMac[6];
					ethLayer->getSourceMac().copyTo(srcMac);
					rtt_t tstmp;
					tstmp = ((uint64_t)srcMac[0]*256*256*256*256*256) + ((uint64_t)srcMac[1]*256*256*256*256) + ((uint64_t)srcMac[2]*256*256*256) + ((uint64_t)srcMac[3]*256*256) + ((uint64_t)srcMac[4]*256) + ((uint64_t)srcMac[5]);
					//printf("%u %u %u %u %u %u\n",*(srcMac), *(srcMac+1), *(srcMac+2), *(srcMac+3), *(srcMac+4), *(srcMac+5));
					
					//printf("timestamp: %lu\n", tstmp);
					pcpp::TcpOption windowScaleOption = tcpLayer->getTcpOption(pcpp::PCPP_TCPOPT_WINDOW);
							
					uint32_t srcIP = ipHeader->ipSrc;
					uint32_t dstIP = ipHeader->ipDst;
					uint16_t srcPort = tcpHeader->portSrc;
					uint16_t dstPort = tcpHeader->portDst;
					// uint8_t protoType = ipHeader->protocol;
					//uint8_t windowScaleFactor;
					
					// FlowKey fkey = {srcIP, dstIP, protoType, srcPort, dstPort};
				    rttKey rkey1 = {srcIP, dstIP, srcPort, dstPort};
					rttKey rkey2 = {dstIP, srcIP, dstPort, srcPort};
					wsKey wkey = {srcIP, srcPort};
					//std::unordered_map<FlowKey, FlowValue>::iterator it;

					if(tcpHeader->synFlag == 1){ // handles SYN, SYN+ACK pkts
						uint8_t* windowScalePtr = windowScaleOption.getValue();
						if(tcpHeader->ackFlag == 0){ // SYN coming from sender; the syn-ack will have random sequence number so we need to verify using ack no.
							
							// FlowValue fvalue = {"syn-sent", tcpHeader->sequenceNumber+1, *windowScalePtr, 0, tcpHeader->windowSize, 0};
							// FLOW_MAP.insert({fkey, fvalue});
							// ACTIVE_FLOWS += 1;
							WS_MAP.insert({wkey, *windowScalePtr});
							RTT_MAP.insert({rkey1, tstmp});
							RTT_MAP.insert({rkey2, tstmp});

							end_time = std::chrono::high_resolution_clock::now();

							duration_usecs = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

							// printf("SYN pkt processing took %llu us.\n", duration_usecs);

							
						} else{ // SYN+ACK coming from receiver
							// FlowKey fkey = {dstIP, srcIP, protoType, dstPort, srcPort};
							// const auto& it = FLOW_MAP.find(fkey);

							// if(it != FLOW_MAP.end() && it->second.connState == "syn-sent" && it->second.expSeqOrAckNo == tcpHeader->ackNumber){ // checking against expected Ack Number
							// 	it->second.connState = "syn-ack-received";
							// 	it->second.expSeqOrAckNo = tcpHeader->ackNumber;
							// 	it->second.receiverRwnd = tcpHeader->windowSize;
							// 	it->second.receiverWS = *windowScalePtr;
							// }
							WS_MAP.insert({wkey, *windowScalePtr});

							end_time = std::chrono::high_resolution_clock::now();

							duration_usecs = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

							// printf("SYN-ACK pkt processing took %llu us.\n", duration_usecs);
						}
					} else if(tcpHeader->finFlag == 1){ // handles FIN pkts from sender ONLY
						// const auto& it = FLOW_MAP.find(fkey);
						// if(it != FLOW_MAP.end() && it->second.connState == "conn-established"){
						// 	it->second.connState = "fin-sent";
						// 	it->second.expSeqOrAckNo = tcpHeader->ackNumber;
						// } 
					} else if(tcpHeader->rstFlag == 1){ // rst can come from any direction
						// const auto& it = FLOW_MAP.find(fkey); // fkey is for pkt going from sender to receiver
						// if(it != FLOW_MAP.end()){
						// 	ACTIVE_FLOWS -= 1;
						// 	FLOW_MAP.erase(fkey);
						// }
						
						// FlowKey fkey = {dstIP, srcIP, protoType, dstPort, srcPort};
						// const auto& itr = FLOW_MAP.find(fkey); // fkey is for pkt going from receiver to sender
						// if(itr != FLOW_MAP.end()){
						// 	ACTIVE_FLOWS -= 1;
						// 	FLOW_MAP.erase(fkey);
						// }
					} else if(tcpHeader->ackFlag == 1){ // handles ACK pkts
						// const auto& it = FLOW_MAP.find(fkey); // fkey is for pkt going from sender to receiver
						// if(it != FLOW_MAP.end()){
						// 	if(it->second.connState == "syn-ack-received" && it->second.expSeqOrAckNo == tcpHeader->sequenceNumber){
						// 		it->second.connState = "conn-established";
						// 		ACTIVE_FLOWS += 1;
						// 	}
						// 	if(it->second.connState == "fin-sent" && it->second.expSeqOrAckNo == tcpHeader->sequenceNumber){
						// 		ACTIVE_FLOWS -= 1;
						// 		FLOW_MAP.erase(fkey);
						// 	}
						// }
						const auto& it1 = RTT_MAP.find(rkey1); // subtract ack_tstmp by syn_tstmp
						const auto& it2 = RTT_MAP.find(rkey2);
						if(it1 != RTT_MAP.end()){ // update RTT map based on RTT reported in ACK
							it1->second = tstmp; // - it1->second;
						}
						if(it2 != RTT_MAP.end()){
							it2->second = tstmp; // - it2->second;
						}

						const auto& ws_src_it = WS_MAP.find({srcIP, srcPort});
						const auto& ws_dst_it = WS_MAP.find({dstIP, dstPort});

						if(ws_src_it != WS_MAP.end()){
							srcWS = ws_src_it->second;
						} else { printf("srcWS not found for this handshake"); exit(1);}
						
						if(ws_dst_it != WS_MAP.end()){
							dstWS = ws_dst_it->second;
						} else { printf("dstWS not found for this handshake"); exit(1);}

						rtt_ws_info.srcIP   = ntohl(srcIP);
						rtt_ws_info.dstIP   = ntohl(dstIP);
						rtt_ws_info.srcPort = ntohs(srcPort);
						rtt_ws_info.dstPort = ntohs(dstPort);
						rtt_ws_info.rtt_mul = tstmp / 10000000;
						rtt_ws_info.srcWS   = srcWS;
						rtt_ws_info.dstWS   = dstWS;

						end_time = std::chrono::high_resolution_clock::now();

						status = bfrt.add_rtt_ws_entry_pair(rtt_ws_info);
						CHECK_BF_STATUS(status);

						bfrt_end_time = std::chrono::high_resolution_clock::now();

						duration_usecs = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
						bfrt_duration_usecs = std::chrono::duration_cast<std::chrono::microseconds>(bfrt_end_time - end_time).count();

						// printf("ACK pcpp pkt processing took %llu us.\n", duration_usecs);
						// printf("ACK bfrt pkt processing took %llu us.\n", bfrt_duration_usecs);

						printf("Added 2 entries successfully to rtt_ws_table\n");

					} else { }
				}
			}
		}
	}

	return true;
}

void ReceiverWorkerThread::stop()
{
	print_rtt_map();
	print_ws_map();

	// printf("Active Flows in stop thread: %d\n", ACTIVE_FLOWS);
	m_Stop = true;
}

uint32_t ReceiverWorkerThread::getCoreId() const
{
	return m_CoreId;
}
