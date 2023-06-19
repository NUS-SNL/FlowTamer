#!/bin/bash

data_dir="${1:-datadump}"

disable_tso_gro() {
    echo "Disabling TSO and GRO"
    sudo ip netns exec ens114f0 sudo ethtool -K ens114f0 tso off
    sudo ip netns exec ens114f0 sudo ethtool -K ens114f0 gro off
    sudo ip netns exec ens114f1 sudo ethtool -K ens114f1 tso off
    sudo ip netns exec ens114f1 sudo ethtool -K ens114f1 gro off
}

ensure_dir_notexist() {
    dir="$1"

    if [ -d $dir ]; then
        echo "Data dump directory already exists. Exiting..."
        exit
    fi

    mkdir $dir
}

single_flow() {
    cca="$1"
    rtt1="$2"
    rtt2="$3"

    owd1=$(($rtt1 / 2))
    owd2=$(($rtt2 / 2))

    iperfres1file="_iperfres1.txt"
    iperfres2file="_iperfres2.txt"
    port1="5001"
    port2="5002"

    echo "Running - RTT: ${rtt1}ms + ${rtt2}ms CCA: ${cca}"

    sudo ip netns exec ens114f1 iperf -s -i 1 -p $port1 -Z $cca > $iperfres1file 2>&1 &
    sudo ip netns exec ens114f1 iperf -s -i 1 -p $port2 -Z $cca > $iperfres2file 2>&1 &
    sudo ip netns exec ens114f1 tcpdump -i ens114f1 -s 80 -w ${data_dir}/recv_${cca}.pcap tcp &
    sudo ip netns exec ens114f0 tcpdump -i ens114f0 -s 80 -w ${data_dir}/post_mm_${cca}.pcap tcp &
    sleep 1

    sudo ip netns exec ens114f0 su --command="mm-delay $owd1 ./_sender.sh $cca $rtt1 $port1 $data_dir" $USER &
    sleep 0.4
    sudo ip netns exec ens114f0 su --command="mm-delay $owd2 ./_sender.sh $cca $rtt2 $port2 $data_dir" $USER &

    sleep 60

    sudo pkill iperf
    sudo pkill --signal SIGINT tcpdump
    sleep 1
    sudo pkill tcpdump
    wait
    stty sane

    echo "==============================================="
    echo "Iperf results for ${rtt1}ms"
    cat $iperfres1file
    echo
    echo "==============================================="
    echo "Iperf results for ${rtt2}ms"
    cat $iperfres2file
    echo
}

plot() {
    ./plot.sh $data_dir
}


disable_tso_gro
ensure_dir_notexist $data_dir
single_flow bbr 20 40
plot
