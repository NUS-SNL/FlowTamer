#About: To be run on the sender VM for starting iperf flow and sstool probing for getting cwnd values

#!/bin/sh
sudo killall iperf3
sudo sysctl net.ipv4.tcp_congestion_control=$1
rm "data/json_files/$2.json"
./ss_probe.sh 15 "data/cwnd_time_raw_files/$2.txt" & # to be executed if you want to plot cwnd time series plot
iperf3 -c 10.1.1.2 -C $1 -t 13 -i 1 -J --logfile "data/json_files/$2" --cport=5200
