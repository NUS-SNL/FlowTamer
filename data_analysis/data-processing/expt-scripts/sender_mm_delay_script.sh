#!/bin/bash

if [ "$#" -ne 3 ];then
    echo "Usage: $0 <cc> <rtt> <duration>"
    exit 1
fi

cc=$1
rtt=$2
duration=$3

sudo sysctl net.ipv4.tcp_timestamps=0

sudo tcpdump -i ingress -s 80 -w sender_${cc}_${rtt}ms.pcap &
sleep 2
echo "Started sender tcpdump..."

echo "Starting tcpflow..."
env TCP_WINDOW_SIZE=8M iperf -c 10.1.1.2 -t $duration -Z $cc
#iperf3 -c 10.1.1.2 -i 1 -t 15 -C $cc

sleep 2
sudo killall tcpdump

