#!/bin/bash

data_dir="$1"

rtt1="20"
rtt2="40"
cca="BBR"
max_thresh="250"

pcap_parser="/home/lingesh/repos/inNetworkCC/data_analysis/pcap-parser/pcap-parser"
process_tx="/home/lingesh/repos/inNetworkCC/data_analysis/trace-analysis/process_tx_trace.py"
process_rx="/home/lingesh/repos/inNetworkCC/data_analysis/trace-analysis/process_rx_trace.py"

analyze() {
    for f in ${data_dir}/*.pcap; do
        $pcap_parser $f
    done

    for f in ${data_dir}/recv*.csv; do
        $process_rx $f
    done

    for f in ${data_dir}/send*.csv; do
        $process_tx $f
    done
}

rename_dat() {
    csv1=$(find $data_dir -name "send_*_${rtt1}ms.csv")
    csv2=$(find $data_dir -name "send_*_${rtt2}ms.csv")
    port1=$(sed -n "2p" $csv1 | awk -F, '{print $9}')
    port2=$(sed -n "2p" $csv2 | awk -F, '{print $9}')

    for f in ${data_dir}/*.dat; do
        mv $f ${f//${port1}/${rtt1}ms} 2>/dev/null
        mv $f ${f//${port2}/${rtt2}ms} 2>/dev/null
    done
}

find_min_ts() {
    csv1=$(find $data_dir -name "send_*_${rtt1}ms.csv")
    csv2=$(find $data_dir -name "send_*_${rtt2}ms.csv")
    ts1=$(sed -n "2p" $csv1 | awk -F, '{print $2}')
    ts2=$(sed -n "2p" $csv2 | awk -F, '{print $2}')

    mints=$(python3 -c "print(min(${ts1}, ${ts2}))")
}

plot() {
    gnuplot -e "filename='${data_dir}/plot.pdf';\
                plottitle='${cca} ${rtt1}ms and ${rtt2}ms - ${max_thresh}KB';\
                datadir='${data_dir}';\
                mints='${mints}';\
                flowth1dat='${data_dir}/flow_th_${rtt1}ms.dat';\
                flowth2dat='${data_dir}/flow_th_${rtt2}ms.dat';\
                flowth1title='Throughput (${rtt1}ms)';\
                flowth2title='Throughput (${rtt2}ms)';\
                flowrwnd1dat='${data_dir}/flow_rwnd_${rtt1}ms.dat';\
                flowrwnd2dat='${data_dir}/flow_rwnd_${rtt2}ms.dat';\
                flowrwnd1title='ReceiverRwnd (${rtt1}ms)';\
                flowrwnd2title='ReceiverRwnd (${rtt2}ms)';\
                algorwnddat='${data_dir}/algo_rwnd.dat';\
                algorwndtitle='AlgoRwnd';\
                qdepthdat='${data_dir}/qdepth.dat';\
                qdepthtitle='qdepth';\
                flowbif1dat='${data_dir}/flow_bif_${rtt1}ms.dat';\
                flowbif2dat='${data_dir}/flow_bif_${rtt2}ms.dat';\
                flowbif1title='flowPiF (${rtt1}ms)';\
                flowbif2title='flowPiF (${rtt2}ms)'" algo_working.gp
    echo "Gnuplotted!"
}


analyze
rename_dat
find_min_ts
plot


