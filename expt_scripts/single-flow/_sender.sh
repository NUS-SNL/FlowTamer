#!/bin/bash

cca="$1"
rtt="$2"

sudo sysctl -w net.ipv4.tcp_timestamps=0

sudo tcpdump -i ingress -s 80 -w send_${cca}_${rtt}ms.pcap tcp &
sleep 1

iperf -c 10.1.1.2 -t 31 -Z $cca -w 8M

sleep 2
sudo pkill --signal SIGINT tcpdump
sleep 1
sudo pkill tcpdump
wait
