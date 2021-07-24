#if !defined(WIN32) && !defined(WINx64)
#include <netinet/in.h> // this is for using ntohs() and htons() on non-Windows OS's
#endif

#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <pcapplusplus/PcapFileDevice.h>
#include <pcapplusplus/InnetworkccInfoLayer.h>
#include <pcapplusplus/TcpLayer.h>
#include <pcapplusplus/IPv4Layer.h>
#include <pcapplusplus/EthLayer.h>
#include <pcapplusplus/Packet.h>


#define RES_CSV_FILE "results.csv"

int main(int argc, char* argv[])
{

    std::string inputFile, outputFile;
    
    /* Args checking */
    if(argc < 2){ // no argument supplied
        printf("Usage: %s <input pcap file> [output dat file]\n", argv[0]);
        exit(1);
    }

    inputFile = std::string(argv[1]);

    if(argc == 2){ // only pcap filename supplied
        size_t pos = inputFile.find(".pcap");

        if(pos == std::string::npos){
            printf("Input pcap file MUST have .pcap extension\n");
            exit(1);
        }

        // input file has a valid extension
        outputFile = inputFile.substr(0, pos) + ".csv";
        
    }
    else if(argc == 3){
        outputFile = argv[2];
    }
    else{
        printf("Invalid number of arguments.\n");
        printf("Usage: %s <input pcap file> [output dat file]\n", argv[0]);
        exit(1);
    }

    printf("Pcap Parsing...\n");
    printf("Input  : %s\n", inputFile.c_str());
    printf("Output : %s\n", outputFile.c_str());


    pcpp::IFileReaderDevice* reader = pcpp::IFileReaderDevice::getReader(inputFile.c_str());

    // verify that a reader interface was indeed created
    if (reader == NULL)
    {
        printf("Cannot determine reader for file type\n");
        exit(1);
    }

    // open the reader for reading
    if (!reader->open())
    {
        printf("Cannot open input.pcap for reading\n");
        exit(1);
    }

    std::ofstream res_csv_file(outputFile.c_str());
    std::string rowEntry = "frame.num,frame.time,frame.time_rel,frame.len,ip.src,ip.dst,tcp.seq,tcp.ack,tcp.srcPort,tcp.dstPort,tcp.rwnd,incc.algo_rwnd,incc.rtt_mul,incc.qdepth_sum,incc.pkt_count,incc.qdepth,incc.final_rwnd,incc.ws";
    res_csv_file << rowEntry.c_str() << "\n";

    int packetCount = 0;
    double initialTimestamp = 0;
    pcpp::RawPacket rawPacket;
    while(reader->getNextPacket(rawPacket)){
        rowEntry = "";
        packetCount = packetCount + 1;
        int frameLen = rawPacket.getFrameLength();
        
        timespec ts = rawPacket.getPacketTimeStamp();
        double timestamp = (double)(ts.tv_sec*1.0 + (ts.tv_nsec/1000000000.0));
        if(packetCount == 1){ initialTimestamp = timestamp; }
        double timeRel = timestamp - initialTimestamp;

        //printf("framelen = %d\ntimestamp = %lf\ntime_rel = %lf\n",frameLen, timestamp, timeRel);
        rowEntry = rowEntry + std::to_string(packetCount) + ",";
        rowEntry = rowEntry + std::to_string(timestamp) + ",";
        rowEntry = rowEntry + std::to_string(timeRel) + ",";
        rowEntry = rowEntry + std::to_string(frameLen) + ",";

        pcpp::Packet parsedPacket(&rawPacket);
        pcpp::TcpLayer* tcpLayer = parsedPacket.getLayerOfType<pcpp::TcpLayer>();
        pcpp::IPv4Layer* ipLayer = parsedPacket.getLayerOfType<pcpp::IPv4Layer>();
        pcpp::tcphdr* tcpHeader = tcpLayer->getTcpHeader();

        if(parsedPacket.isPacketOfType(pcpp::TCP)){
            pcpp::IPv4Address srcIP = ipLayer->getSrcIPv4Address();
            pcpp::IPv4Address dstIP = ipLayer->getDstIPv4Address();

            // printf("srcIP: %s\ndstIP: %s\n",srcIP.toString().c_str(), dstIP.toString().c_str());

            uint16_t srcPort = tcpLayer->getSrcPort();
            uint16_t dstPort = tcpLayer->getDstPort();
            uint32_t seqNumber = ntohl(tcpHeader->sequenceNumber);
            uint32_t ackNumber = ntohl(tcpHeader->ackNumber);
            uint16_t rwnd = ntohs(tcpHeader->windowSize);
            //uint16_t len = ntohs(tcpHeader->dataOffset);

            // printf("srcPort = %d\ndstPort = %d\nseqNumber = %u\nackNumber = %u\n",srcPort, dstPort, seqNumber, ackNumber);
            rowEntry = rowEntry + srcIP.toString().c_str() + ",";
            rowEntry = rowEntry + dstIP.toString().c_str() + ",";
            rowEntry = rowEntry + std::to_string(seqNumber) + ",";
            rowEntry = rowEntry + std::to_string(ackNumber) + ",";
            rowEntry = rowEntry + std::to_string(srcPort) + ",";
            rowEntry = rowEntry + std::to_string(dstPort) + ",";
            rowEntry = rowEntry + std::to_string(rwnd);
        }
        if(parsedPacket.isPacketOfType(pcpp::InnetworkCCInfo)){
            pcpp::InnetworkccInfoLayer *innetworkccInfoLayer = parsedPacket.getLayerOfType<pcpp::InnetworkccInfoLayer>();
            uint32_t algo_rwnd = innetworkccInfoLayer->getAlgo_rwnd();
            uint32_t rtt_mul = innetworkccInfoLayer->getRtt_mul();
            uint32_t qdepth_sum = innetworkccInfoLayer->getQdepth_sum();
            uint32_t pkt_count = innetworkccInfoLayer->getPkt_count();
            uint32_t qdepth = innetworkccInfoLayer->getQdepth();
            uint16_t final_rwnd = innetworkccInfoLayer->getFinal_rwnd();
            uint16_t ws = innetworkccInfoLayer->getWs();

            // printf("algo_rwnd: %d\nrtt_mul: %d\nqdepth_sum: %d\npkt_count: %d\nqdepth: %d\nfinal_rwnd: %d\nws: %d\n\n", algo_rwnd, rtt_mul, qdepth_sum, pkt_count, qdepth, final_rwnd, ws);
            rowEntry = rowEntry + ",";
            rowEntry = rowEntry + std::to_string(algo_rwnd) + ",";
            rowEntry = rowEntry + std::to_string(rtt_mul) + ",";
            rowEntry = rowEntry + std::to_string(qdepth_sum) + ",";
            rowEntry = rowEntry + std::to_string(pkt_count) + ",";
            rowEntry = rowEntry + std::to_string(qdepth) + ",";
            rowEntry = rowEntry + std::to_string(final_rwnd) + ",";
            rowEntry = rowEntry + std::to_string(ws) + "\n";        
        } else {
            rowEntry = rowEntry + "\n";
        }
        res_csv_file << rowEntry;
    }

    printf("Total packets parsed: %d\n", packetCount);

    res_csv_file.close();
    reader->close();

    return 0;
}





