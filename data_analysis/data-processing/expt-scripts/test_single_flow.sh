#!/bin/bash


if [ "$#" -lt 2 ];then
    echo "Usage: $0 <cc> <rtt> [duration]"
    exit 1
fi

cc=$1
rtt=$2

if [ "$#" -eq 3 ];then
    duration=$3
else
    duration=31
fi


owd=$(($rtt / 2))

echo "Running for \"$cc\" - ${rtt}ms"

sudo ip netns exec h2 env TCP_WINDOW_SIZE=6M iperf -s -i 1 &
sudo ip netns exec h2 tcpdump -i ens1f1 -s 80 -w receiver_${cc}_${rtt}ms.pcap &

mm-delay $owd ./sender_mm_delay_script.sh $cc $rtt $duration

sudo killall iperf


