#!/bin/bash

sudo timeout 5 tcpdump -n -i ens81f0 -w out.pcap &
echo hi
sleep 1
echo hi
wait
