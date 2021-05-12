#!/usr/bin/env python3
# generates the .csv files with timestamp, mss, cwnd
# before running this script manually cut the initial few seconds part and the last few seconds part
# the format of txt file data must be as follows
"""
time=1617613784.009455169  
 
mss:1448 cwnd:789
time=1617613784.012863363  
 
mss:1448 cwnd:791
time=1617613784.016347115  
 
mss:1448 cwnd:792
"""

txt_file = open(sys.argv[1], "r")
csv_file = open(sys.argv[2], "w")

lines = txt_file.readlines()
index = 0
csv_file.write("timestamp_sec,mss_bytes,cwnd_mss\n")
while index < len(lines):
    if lines[index][0:5] == "time=":
        timestamp = lines[index][5:25]
        mss_cwnd_list = lines[index+2].split()
        mss = mss_cwnd_list[0][4:]
        cwnd = mss_cwnd_list[1][5:]
        csv_file.write(timestamp + ',' + mss + ',' + cwnd + '\n')
        index = index + 3

txt_file.close()
csv_file.close()
