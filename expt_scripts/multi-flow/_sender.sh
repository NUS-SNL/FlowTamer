#!/bin/bash

cca="$1"
rtt="$2"
port="$3"
data_dir="$4"

sudo sysctl -w net.ipv4.tcp_timestamps=0

sudo timeout 45 tcpdump -i ingress -s 80 -w ${data_dir}/send_${cca}_${rtt}ms.pcap tcp &
sleep 2

iperf -c 10.1.1.2 -t 31 -Z $cca -w 8M -p $port

wait
