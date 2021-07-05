#!/usr/bin/env python3
import csv
import sys
import os

WINDOW_TIME_LENGTH = 0.001
DST_IP = "10.1.1.2"

class PerFlowThroughput:
    def __init__(self, src_port, cc_name, output_dir):
        self.src_port = src_port
        self.cc_name = cc_name
        self.moving_window = []  # (time, pkt_size)
        self.rate_timeseries = [] # (time, avg rate)
        self.window_sum = 0
        self.start_time = 0
        filename = output_dir + "/flow_th_{}.dat".format(src_port)
        self.outfile = open(filename, "w")
    
    def __del__(self):
        self.outfile.close()


    def add_packet(self, time, pkt_size):
        self.moving_window.append((time, pkt_size))
        self.window_sum += pkt_size
        end_time = time # self.moving_window[-1][0]
        curr_window_len = end_time - self.start_time
        
        if(curr_window_len > WINDOW_TIME_LENGTH): 
            # calculate rate for this window
            rate = self.window_sum / curr_window_len
            rate = (rate * 8 / 1000000) # convert to Mbps
            self.outfile.write("{} {:.6f}\n".format(end_time, round(rate, 6)))
            # self.rate_timeseries.append((end_time, round(rate, 6)))

            # move the window forward
            removed_pkt = self.moving_window.pop(0)
            self.start_time = self.moving_window[0][0]
            self.window_sum -= removed_pkt[1]

    def get_overall_average(self):
        sum = 0
        count = 0
        for data_point in self.rate_timeseries:
            sum += data_point[1]
            count += 1
        return sum / count


def main():
    if len(sys.argv) != 2:
        print("Usage: {} <csv file>".format(sys.argv[0]))
        sys.exit(1)

    csv_path = os.path.abspath(sys.argv[1])
    outdir = os.path.dirname(csv_path)

    print("Processing csv file:", csv_path)
    
    csv_file = open(csv_path, 'r')
    csv_reader = csv.reader(csv_file)
    header = next(csv_reader)

    flows = set()
    flow_throughputs = {}

    pkt_count = 0
    for row in csv_reader:
        pkt_count += 1
        row_len = len(row)
        if(row_len >= 10): # valid TCP pkt
            rel_time = float(row[2])
            pktlen = int(row[3])
            dst_ip = row[5]
            src_port = int(row[8])

            if dst_ip != DST_IP: # consider one direction only
                continue

            if src_port not in flows:
                flows.add(src_port)
                curr_flow_throughput = PerFlowThroughput(src_port, 'cubic', outdir)
                flow_throughputs[src_port] = curr_flow_throughput
            else:
                curr_flow_throughput = flow_throughputs[src_port]

            curr_flow_throughput.add_packet(rel_time, pktlen)


            if(row_len == 17):
                # extract inNetworkCC Info
                algo_rwnd = int(row[10])
                rtt_mul = int(row[11])
                qdepth_sum = int(row[12])
                qdepth_pkt_count = int(row[13])
                qdepth = int(row[14])
                final_rwnd = int(row[15])
                ws = int(row[16])

                # TODO process the inNetworkCC info

        
    
    print("Processed {} packets".format(pkt_count))
    print("Flow set is:", flows)
"""     for flow in flows:
        flow_throughput = flow_throughputs[flow]
        flow_th_file = open("flow_th_{}.dat".format(flow), "w")
        print("Flow: {}, Avg Throughput: {}".format(flow, flow_throughput.get_overall_average()))
        for data_point in flow_throughput.rate_timeseries:
            flow_th_file.write("{} {}\n".format(data_point[0], data_point[1]))
        flow_th_file.close() """





if __name__ == "__main__":
    main()
