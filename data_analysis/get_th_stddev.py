# About: Computes that standard deviation of per-flow throughput
#!/usr/bin/env python3

import sys
import csv

CC_ALGO = sys.argv[1]
START_RWND = int(sys.argv[2])
END_RWND = int(sys.argv[3])
RWND_INC_DEC = int(sys.argv[4])
TRIALS = int(sys.argv[5])

DST_IP = "10.1.1.2"
DST_PORT = 5201
SRC_PORT = 5200
WINDOW_TIME_LENGTH = 0.01
MOVING_STRIDE = 1
START_TIME = 2.0
END_TIME = 12.0

MIN_MAX_STATS_FILE = 
RES_FILE = '/home/harsh/data/th-variances/' + CC_ALGO + '_avg_th_stddev.txt'
def get_flow_times_and_sizes(csv_file_name):
    flow_times = []
    flow_sizes = []
    with open(csv_file_name, 'r') as csv_file:
        input_csv = csv.reader(csv_file)
        next(input_csv)  # skip the header
        for packet in input_csv:
            packet_dst_ip = packet[4]
            packet_dst_port = int(packet[5])  # one port, one flow
            packet_src_port = int(packet[9])
            packet_time = float(packet[10])
            packet_size = float(packet[2])
            if packet_dst_ip == DST_IP and packet_dst_port == DST_PORT and packet_src_port == SRC_PORT and packet_time>=START_TIME and packet_time<=END_TIME:  # appending only the relevant time and packet size information
                flow_times.append(packet_time)
                flow_sizes.append(packet_size)
    return flow_times, flow_sizes


# packet sizes and packet timestamps -> rate, timestamps
#                                     [List]      [List]       int (~ 1 RTT)    
def get_moving_window_average_rates(flow_times, flow_sizes, window_time_length, moving_stride=1):
    rate_times = []
    rate_values = []
    window_start_time = 0
    window_size_sum = 0
    window_start_pointer = 0
    window_end_pointer = 0
    for time in flow_times:
        window_size_sum += flow_sizes[window_end_pointer]
        window_end_pointer += 1
        if time - window_start_time > window_time_length:
            rate_times.append(time)
            window_time_sum = time - window_start_time
            window_rate = window_size_sum / window_time_sum
            rate_values.append(window_rate)
            # push window forward with strides
            window_size_sum -= flow_sizes[window_start_pointer]
            window_start_pointer += moving_stride
            window_start_time = flow_times[window_start_pointer]
    # converting rate to Mbps
    rate_values = [float(8 * i / 1000000) for i in rate_values]
    # list with rate, times. i.e at time[i], the rate was rate[i]
    return rate_times, rate_values

def get_stddev(rate_times, rate_values):
    total = 0
    for rate in rate_values:
        total = total + rate
    mean = total/len(rate_values)
    count = 0
    total = 0
    min_rate = 9999
    max_rate = -999
    for rate in rate_values:
        if(rate > max_rate):
            max_rate = rate
        if(rate < min_rate):
            min_rate = rate
        count = count + 1
        total = total + ((rate - mean)**2)
    stddev = (total/count)**0.5
    return stddev, min_rate, max_rate
######################
#MAIN SCRIPT
#######################
res_file = open(RES_FILE, "w")
res_file.close()
res_file = open(RES_FILE, "a+")

for rwnd in range(START_RWND, END_RWND, RWND_INC_DEC):
    total_stddev = 0.0
    for trial in range(TRIALS):
        csv_file_name = "/home/harsh/data/pcap_csv_files/" + CC_ALGO + "_" + str(rwnd) + "_t" + str(trial) + ".pcap.csv"
        flow_times, flow_sizes = get_flow_times_and_sizes(csv_file_name)
        rate_times, rate_values = get_moving_window_average_rates(flow_times, flow_sizes, WINDOW_TIME_LENGTH, MOVING_STRIDE)
        stddev,min_rate,max_rate = get_stddev(rate_times, rate_values)
        total_stddev = total_stddev + stddev
    avg_stddev = (total_stddev/3.0)
    res_file.write(str(rwnd*128.0/1000) + " " + str(avg_stddev) + "\n")
