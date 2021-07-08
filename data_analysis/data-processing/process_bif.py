#!/usr/bin/env python3
import csv
import sys
import os

RECEIVER_IP = "10.1.1.2"

BIF_WINDOW_TIME_LENGTH = 0.005 # 5ms

class BytesInFlightTracker:
    def __init__(self, src_port, seq_no, output_dir):
        self.port = src_port
        self.moving_window = []  # (time, bif)
        self.window_sum = 0
        self.window_count = 0
        self.start_time = 0  # just declaration
        self.is_first_pkt = True
        self.tx_bytes = 0
        self.acked_bytes = seq_no
        filename = output_dir + "/flow_bif_{}.dat".format(src_port)
        self.outfile = open(filename, "w")

    def __del__(self):
        self.outfile.close()

    def add_to_moving_window(self, time, bif):
        self.moving_window.append((time, bif))
        self.window_sum += bif
        self.window_count += 1
        end_time = time # self.moving_window[-1][0]

        if self.is_first_pkt == True:
            self.start_time = time
            self.is_first_pkt = False
            return

        curr_window_len = end_time - self.start_time
        if(curr_window_len > BIF_WINDOW_TIME_LENGTH): 
            # calculate average qdepth for this window
            avg_bif = self.window_sum / float(self.window_count)
            self.outfile.write("{} {}\n".format(end_time, round(avg_bif)))

            # move the window forward
            removed_pkt = self.moving_window.pop(0)
            self.start_time = self.moving_window[0][0]
            self.window_sum -= removed_pkt[1]
            self.window_count -= 1


    def update_tx_bytes(self, time, frame_len, seq_no):
        if frame_len == 52:  # SYN
            new_tx_bytes = seq_no
        elif frame_len == 40: # ACK of handshake/teardown
            new_tx_bytes = seq_no - 1
        else:
            payload = frame_len - 40 # assuming sender-side IP trace
            new_tx_bytes = seq_no + payload -1
        if new_tx_bytes > self.tx_bytes: # not a ReTx pkt
            self.tx_bytes = new_tx_bytes
            bif = self.tx_bytes - self.acked_bytes
            # self.outfile.write("{} {}\n".format(time, bif))
            self.add_to_moving_window(time, bif)


    def update_acked_bytes(self, time, ack_no):
        # TODO: check for dup ack_no. Should be SACK. Use right edge instead
        self.acked_bytes = ack_no - 1
        bif = self.tx_bytes - self.acked_bytes
        if bif == -1: # handling last ACK
            bif = 0
        # self.outfile.write("{} {}\n".format(time, bif))
        self.add_to_moving_window(time, bif)
    


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

    bif_tracker = None

    pkt_count = 0
    for row in csv_reader:
        pkt_count += 1
        row_len = len(row)
        if(row_len >= 10): # valid TCP pkt
            time = float(row[1])
            pktlen = int(row[3])
            src_ip = row[4]
            dst_ip = row[5]
            seq_no = int(row[6])
            ack_no = int(row[7])
            src_port = int(row[8])
            dst_port = int(row[9])
            
            if dst_ip != RECEIVER_IP and src_ip != RECEIVER_IP: # random pkt
                continue

            # Not a random pkt
            if dst_ip == RECEIVER_IP: # data pkt
                if bif_tracker == None: # expect 1st valid pkt to be SYN
                    bif_tracker = BytesInFlightTracker(src_port, seq_no, outdir)  
                bif_tracker.update_tx_bytes(time, pktlen, seq_no)
            
            elif src_ip == RECEIVER_IP:  # ack pkt
                bif_tracker.update_acked_bytes(time, ack_no)

if __name__ == "__main__":
    main()
