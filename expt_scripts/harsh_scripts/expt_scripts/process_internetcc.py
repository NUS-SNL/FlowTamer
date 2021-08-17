# About: Automation framework for measuring throughput for various rwnd values
#!/usr/bin/env python3

import os
import sys
import time
import subprocess
from colorit import *
sys.path.append("/home/cirlab/jarvis-tofino/harsh-tofino/internet_cc")

CC_ALGO = sys.argv[1]
START_RWND = int(sys.argv[2])
END_RWND = int(sys.argv[3])
RWND_INC_DEC = int(sys.argv[4])
NUM_TRIALS = int(sys.argv[5])
HALF_RTT = 5

def capture_trace(pcap_file):
    subcmd = 'sudo tcpdump -i enp2s0 -w {} tcp -s 84'.format(pcap_file)
    cmd = "ssh internetcc1 '{}' &".format(subcmd)
    print(cmd)
    subprocess.call(cmd, shell=True)

def kill_tcpdump():
    subcmd = 'sudo killall tcpdump'
    cmd = "ssh internetcc1 '{}'".format(subcmd)
    subprocess.call(cmd, shell=True)

def run_receiver_script(cc_algo):
    subcmd = './receive.sh {}'.format(cc_algo)
    cmd = "ssh internetcc2 '{}'".format(subcmd)
    subprocess.call(cmd, shell=True)
    time.sleep(0.5)

def run_sender_script(cc_algo, rtt, res_file_name):
    subcmd = 'mm-delay {} ./send.sh {} {}'.format(rtt, cc_algo, res_file_name)
    cmd = "ssh internetcc1 '{}'".format(subcmd)
    subprocess.call(cmd, shell=True)
    time.sleep(0.5)

def convert_to_csv(file_name):
    subcmd = "sudo ./pcap2csv.sh {}".format(file_name)
    cmd = "ssh internetcc1 '{}'".format(subcmd)
    subprocess.call(cmd, shell=True)
    time.sleep(0.5)

def set_rwnd(rwnd):
    print(color_ansi("Setting RWND to {}".format(rwnd), ColorsFG.Yellow))
    set_rwnd_file = "/home/cirlab/jarvis-tofino/harsh-tofino/internet_cc/set_rwnd.py"
    f1 = open(set_rwnd_file, "w")
    cmd = "bfrt.internet_cc.pipe.SwitchIngress.new_rwnd.mod(register_index=0, f1={})".format(rwnd)
    f1.write(cmd)
    f1.close()

    cmd = "/home/cirlab/bf-sde-9.2.0/run_bfshell.sh -b ~/jarvis-tofino/harsh-tofino/internet_cc/set_rwnd.py"
    subprocess.call(cmd, shell=True)
    time.sleep(0.5)

def get_avg_stddev():
    subcmd = 'sudo ./get_th_stddev.py {} {} {} {} {}'.format(CC_ALGO, START_RWND, END_RWND, RWND_INC_DEC, NUM_TRIALS)
    cmd = "ssh internetcc1 '{}'".format(subcmd)
    subprocess.call(cmd, shell=True)

############ MAIN SCRIPT  ################
if(RWND_INC_DEC > 0):
    END_RWND = END_RWND + 1
else:
    END_RWND = END_RWND - 1

for rwnd in range(START_RWND, END_RWND, RWND_INC_DEC):
    for trial in range(NUM_TRIALS):
        print(color_ansi("Starting trial {} for RWND equal to {}".format(trial, rwnd), ColorsFG.Red))
        set_rwnd(rwnd)
        run_receiver_script(CC_ALGO)
        
        #pcap_file_name = "/home/harsh/data/pcap_csv_files/" + CC_ALGO + "_" + str(rwnd) + "_t" + str(trial) + ".pcap"
        #capture_trace(pcap_file_name)
        
        res_file_name = CC_ALGO + "_" + str(rwnd) + "_t" + str(trial)
        run_sender_script(CC_ALGO, HALF_RTT, res_file_name)
        #kill_tcpdump()
        #convert_to_csv(pcap_file_name)

        print(color_ansi("Completed trial {} for RWND equal to {}".format(trial, rwnd), ColorsFG.Green))
get_avg_stddev()        
