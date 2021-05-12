#!/bin/sh
sudo killall iperf3
sudo killall mm-delay
sudo sysctl net.ipv4.tcp_congestion_control=$1
echo "starting iperf3 server"
iperf3 -s -D

############
# Below code needed to measure throughput from the packet trace
############
##################
# To be run on internetcc2 VM for starting iperf server
##################
