#!/bin/bash

pcapParser='/home/raj/Dropbox/NUS-PhD/tofino-p4/harsh-tofino/inNetworkCC/data_analysis/pcap-parser/pcap-parser'
processTxTrace='/home/raj/Dropbox/NUS-PhD/tofino-p4/harsh-tofino/inNetworkCC/data_analysis/data-processing/process_tx_trace.py'
processRxTrace='/home/raj/Dropbox/NUS-PhD/tofino-p4/harsh-tofino/inNetworkCC/data_analysis/data-processing/process_rx_trace.py'

${pcapParser} ./*sender*.pcap
${pcapParser} ./*receiver*.pcap
${processTxTrace} ./*sender*.csv
${processRxTrace} ./*receiver*.csv
