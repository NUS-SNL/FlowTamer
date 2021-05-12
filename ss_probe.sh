#!/bin/sh

initial=$(date +'%s')
now=$initial
rm $2
while [ $now -lt $(($initial + $1)) ]
do
	ss_output=$(ss src 100.64.0.2:5200 dst 10.1.1.2:5201 -i | cut -d ' ' -f 6,10) # before running exp, do mm-delay 5 and check the src ip
	echo "time=$(date +'%s.%N') $ss_output" >> $2
	now=$(date +'%s')
done
