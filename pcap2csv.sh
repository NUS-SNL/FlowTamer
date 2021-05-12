if [ $# -eq 0 ]
then
echo "This scripts converts .pcap files to a more parse-able .csv format." 
echo "To convert X.pcap to X.csv, run './pcap2csv.sh X' "
exit
fi


echo -e "[${green}Converting recv data to .csv format and then delete pcaps${plain}]"
tshark -r $1 -T fields -o "gui.column.format:\"Time\",\"%Aut\"" -e frame.number -e _ws.col.Time -e frame.len -e tcp.seq -e ip.dst -e tcp.dstport -e tcp.time_relative -e tcp.len -e tcp.ack -e tcp.srcport -e frame.time_relative -E header=y -E separator=, -E quote=d -E occurrence=f > $1.csv
