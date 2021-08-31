#!/bin/bash

pcapParser='/home/raj/Dropbox/NUS-PhD/tofino-p4/harsh-tofino/inNetworkCC/data_analysis/trace-analysis/pcap-parser'
processTxTrace='/home/raj/Dropbox/NUS-PhD/tofino-p4/harsh-tofino/inNetworkCC/data_analysis/trace-analysis/process_tx_trace.py'
processRxTrace='/home/raj/Dropbox/NUS-PhD/tofino-p4/harsh-tofino/inNetworkCC/data_analysis/trace-analysis/process_rx_trace.py'

${pcapParser} ./*sender*.pcap
${pcapParser} ./*receiver*.pcap
${processTxTrace} ./*sender*.csv
${processRxTrace} ./*receiver*.csv
