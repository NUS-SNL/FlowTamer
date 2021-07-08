#!/usr/bin/env python3
import csv
import sys
import os

RECEIVER_IP = "10.1.1.2"


class BytesInFlightTracker:
    def __init__(self, src_port, output_dir):
        self.port = src_port
        self.tx_bytes = 0
        self.acked_bytes = 0
        filename = output_dir + "/flow_bif_{}.dat".format(src_port)
        self.outfile = open(filename, "w")

    def __del__(self):
        self.outfile.close()

    def update_tx_bytes(self, time, frame_len, seq_no):
        if frame_len <= 52:  # Packet without payload
            new_tx_bytes = seq_no
        else:
            payload = frame_len - 40 # assuming sender-side IP trace
            new_tx_bytes = seq_no + payload -1
        if new_tx_bytes > self.tx_bytes: # not a ReTx pkt
            self.tx_bytes = new_tx_bytes
            bif = self.tx_bytes - self.acked_bytes
            # self.outfile.write("{} {}\n".format(time, bif))

    def update_acked_bytes(self, time, ack_no):
        # TODO: check for dup ack_no. Should be SACK. Use right edge instead
        self.acked_bytes = ack_no - 1
        bif = self.tx_bytes - self.acked_bytes
        if bif == -1: # handling last ACK
            bif = 0
        self.outfile.write("{} {}\n".format(time, bif))
    


def main():
    if len(sys.argv) != 3:
        print("Usage: {} <csv file> <src port>".format(sys.argv[0]))
        sys.exit(1)

    csv_path = os.path.abspath(sys.argv[1])
    src_port = int(sys.argv[2])
    outdir = os.path.dirname(csv_path)

    print("Processing csv file:", csv_path)
    
    csv_file = open(csv_path, 'r')
    csv_reader = csv.reader(csv_file)
    header = next(csv_reader)

    bif_tracker = BytesInFlightTracker(src_port, outdir)

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
            

            if dst_ip == RECEIVER_IP: # data pkt
                bif_tracker.update_tx_bytes(time, pktlen, seq_no)
            
            elif src_ip == RECEIVER_IP: 
                bif_tracker.update_acked_bytes(time, ack_no)
                
            else: # random pkt
                continue

if __name__ == "__main__":
    main()
