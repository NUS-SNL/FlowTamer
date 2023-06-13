#!/bin/bash

disable_tso_gro() {
    echo "Disabling TSO and GRO"
    sudo ip netns exec ens114f0 sudo ethtool -K ens114f0 tso off
    sudo ip netns exec ens114f0 sudo ethtool -K ens114f0 gro off
    sudo ip netns exec ens114f1 sudo ethtool -K ens114f1 tso off
    sudo ip netns exec ens114f1 sudo ethtool -K ens114f1 gro off
}

single_flow() {
    cca="$1"
    rtt="$2"

    owd=$(($rtt / 2))

    echo "Running - RTT: ${rtt}ms CCA: ${cca}"

    sudo ip netns exec ens114f1 iperf -s -i 1 &
    sudo ip netns exec ens114f1 tcpdump -i ens114f1 -s 80 -w recv_${cca}_${rtt}ms.pcap tcp &
    sudo ip netns exec ens114f0 tcpdump -i ens114f0 -s 80 -w physend_${cca}_${rtt}ms.pcap tcp &
    sleep 1

    sudo ip netns exec ens114f0 su --command="mm-delay $owd ./_sender.sh $cca $rtt" $USER

    sudo pkill iperf
    stty sane
}

disable_tso_gro
single_flow cubic 20
