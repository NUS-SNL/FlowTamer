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

class PerFlowRwndTracker:
    def __init__(self, src_port, ws, output_dir, time, final_rwnd):
        self.src_port = src_port
        self.ws = ws
        self.prev_rwnd = final_rwnd * pow(2, self.ws)
        self.prev_rwnd_time = time
        filename = output_dir + "/flow_rwnd_{}.dat".format(src_port)
        self.outfile = open(filename, "w")
        self.outfile.write("{} {}\n".format(self.prev_rwnd, self.prev_rwnd_time))
        
    def __del__(self):
        # print out the latest seen rwnd (ending)
        self.outfile.write("{} {}\n".format(self.prev_rwnd, self.prev_rwnd_time))
        self.outfile.close()
        
    def track(self, time, final_rwnd):
        if(self.prev_rwnd == final_rwnd): # no change. Update time
            self.prev_rwnd_time = time
        else: # rwnd has changed
            # write the latest timestamped prev_rwnd
            self.outfile.write("{} {}\n".format(self.prev_rwnd, self.prev_rwnd_time))
            # update prev_rwnd
            self.prev_rwnd = final_rwnd * pow(2, self.ws)
            self.prev_rwnd_time = time
            # write it out as well
            self.outfile.write("{} {}\n".format(self.prev_rwnd, self.prev_rwnd_time))

class AlgoRwndTracker:
    def __init__(self, time, algo_rwnd, output_dir):
        self.prev_algo_rwnd = algo_rwnd
        self.prev_algo_rwnd_time = time
        filename = output_dir + "/algo_rwnd.dat"
        self.outfile = open(filename, "w")
        self.outfile.write("{} {}\n".format(self.prev_algo_rwnd, self.prev_algo_rwnd_time))
        
    def __del__(self):
        # print out the latest seen rwnd (ending)
        self.outfile.write("{} {}\n".format(self.prev_algo_rwnd, self.prev_algo_rwnd_time))
        self.outfile.close()
    
    def track(self, time, algo_rwnd):
        if(self.prev_algo_rwnd == algo_rwnd): # no change. Update time
            self.prev_algo_rwnd_time = time
        else: # rwnd has changed
            # write the latest timestamped prev_rwnd
            self.outfile.write("{} {}\n".format(self.prev_algo_rwnd, self.prev_algo_rwnd_time))
            # update prev_rwnd
            self.prev_algo_rwnd = algo_rwnd
            self.prev_algo_rwnd_time = time
            # write it out as well
            self.outfile.write("{} {}\n".format(self.prev_algo_rwnd, self.prev_algo_rwnd_time))

class AlgoQdepthTracker:
    def __init__(self, output_dir):
        self.prev_qdepth_sum = 0
        self.prev_pkt_count = 0
        self.prev_time = 0
        filename = output_dir + "/algo_qdepth.dat"
        self.outfile = open(filename, "w")

    def __del__(self):
        self.outfile.close()

    def track(self, time, qdepth_sum, pkt_count):
        if(pkt_count == 1): # working copy has changed
            avg_qdepth = float(self.prev_qdepth_sum * 80) / self.prev_pkt_count
            self.outfile.write("{} {}\n".format(self.prev_time, round(avg_qdepth)))
        
        self.prev_time = time
        self.prev_qdepth_sum = qdepth_sum
        self.prev_pkt_count = pkt_count
        

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

    flow_throughputs = {}
    flow_rwnds = {}

    algo_rwnd_tracker = None
    algo_qdepth_tracker = AlgoQdepthTracker(outdir)
    
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

            # Throughput processing 
            if src_port not in flow_throughputs:
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

                # per-flow rwnd processing
                if src_port not in flow_rwnds:
                    curr_flow_rwnd_tracker = PerFlowRwndTracker(src_port, ws, outdir, rel_time, final_rwnd)
                    flow_rwnds[src_port] = curr_flow_rwnd_tracker
                else:
                    curr_flow_rwnd_tracker = flow_rwnds[src_port]
                curr_flow_rwnd_tracker.track(rel_time, final_rwnd)

                # algo rwnd processing
                if algo_rwnd_tracker == None:
                    algo_rwnd_tracker = AlgoRwndTracker(rel_time, algo_rwnd, outdir)
                else:
                    algo_rwnd_tracker.track(rel_time, algo_rwnd)

                # algo qdepth processing
                algo_qdepth_tracker.track(rel_time, qdepth_sum, qdepth_pkt_count)

        
    
    print("Processed {} packets".format(pkt_count))
    
"""     for flow in flows:
        flow_throughput = flow_throughputs[flow]
        flow_th_file = open("flow_th_{}.dat".format(flow), "w")
        print("Flow: {}, Avg Throughput: {}".format(flow, flow_throughput.get_overall_average()))
        for data_point in flow_throughput.rate_timeseries:
            flow_th_file.write("{} {}\n".format(data_point[0], data_point[1]))
        flow_th_file.close() """





if __name__ == "__main__":
    main()
