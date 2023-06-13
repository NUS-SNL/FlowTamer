#!/bin/bash

pcap_parser="/home/lingesh/repos/inNetworkCC/data_analysis/pcap-parser/pcap-parser"
process_tx="/home/lingesh/repos/inNetworkCC/data_analysis/trace-analysis/process_tx_trace.py"
process_rx="/home/lingesh/repos/inNetworkCC/data_analysis/trace-analysis/process_rx_trace.py"

$pcap_parser send_cubic_20ms.pcap
$pcap_parser recv_cubic_20ms.pcap
$process_tx send_cubic_20ms.csv
$process_rx recv_cubic_20ms.csv
