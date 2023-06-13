import socket
import os
import sys

hostname = socket.gethostname()


CPU_ETHERNET_DEV_PORT_ENP4S0F1 = 64
CPU_ETHERNET_DEV_PORT_ENP4S0F0 = 66

# Configure front-panel ports
fp_ports = []

if hostname == 'tofino1a':
    fp_ports = [1]
elif hostname == 'tofino1b':
    fp_ports = [31]
elif hostname == 'tofino1c':
    fp_ports = [1]
    fp_ports_40G = [31, 32]

python_lib_append = ""

if hostname == 'tofino1a' or hostname == 'tofino1b' or hostname == 'tofino1c':
    python_lib_append = '/home/cirlab/jarvis-tofino/harsh-tofino/inNetworkCC/control_plane/setup_scripts'
elif hostname == 'hep':
    python_lib_append = '/home/tofino/jarvis-tofino/inNetworkCC/control_plane/setup_scripts'
sys.path.append(python_lib_append)
from LookUpTable import *

if hostname != 'hep':
    for fp_port in fp_ports:
        for lane in range(4):
            dp = bfrt.port.port_hdl_info.get(CONN_ID=fp_port, CHNL_ID=lane, print_ents=False).data[b'$DEV_PORT']
            bfrt.port.port.add(DEV_PORT=dp, SPEED='BF_SPEED_10G', FEC='BF_FEC_TYP_NONE', AUTO_NEGOTIATION='PM_AN_FORCE_DISABLE', PORT_ENABLE=True)

    for fp_port in fp_ports_40G:
        dp = bfrt.port.port_hdl_info.get(CONN_ID=fp_port, CHNL_ID=0, print_ents=False).data[b'$DEV_PORT']
        bfrt.port.port.add(DEV_PORT=dp, SPEED='BF_SPEED_40G', FEC='BF_FEC_TYP_NONE', AUTO_NEGOTIATION='PM_AN_FORCE_DISABLE', PORT_ENABLE=True)

# Add CPU Ethernet ports separately
if hostname == 'tofino1b' or hostname == 'tofino1c':
    for lane in range(4):
        dp = bfrt.port.port_hdl_info.get(CONN_ID=33, CHNL_ID=lane, print_ents=False).data[b'$DEV_PORT']
        bfrt.port.port.add(DEV_PORT=dp, SPEED='BF_SPEED_10G', FEC='BF_FEC_TYP_NONE', PORT_ENABLE=True)

# Add entries to the l2_forward table
l2_forward = bfrt.inNetworkCC.pipe.SwitchIngress.l2_forward
if hostname == 'tofino1b':
    l2_forward.add_with_forward(dst_addr=0x6cb3115309b0, port=128)
    l2_forward.add_with_forward(dst_addr=0x6cb3115309b2, port=129)
    l2_forward.add_with_forward(dst_addr=0x6cb31153099c, port=130)
    l2_forward.add_with_forward(dst_addr=0x6cb31153099e, port=131)
elif hostname == 'tofino1c' or hostname == 'hep':
    l2_forward.add_with_forward(dst_addr=0x3cfdfead82e0, port=132)
    l2_forward.add_with_forward(dst_addr=0x3cfdfead82e1, port=133)
    l2_forward.add_with_forward(dst_addr=0x649d99b16814, port=128)
    l2_forward.add_with_forward(dst_addr=0x649d99b16815, port=136)

# if hostname == 'hep':
#     l2_forward.add_with_forward(dst_addr=0xd606fd18b97a, port=128)
#     l2_forward.add_with_forward(dst_addr=0xee97c394935a, port=129)


# Setup ARP broadcast for the active dev ports
active_dev_ports = [132, 133, 128, 136]
bfrt.pre.node.add(MULTICAST_NODE_ID=0, MULTICAST_RID=0, MULTICAST_LAG_ID=[], DEV_PORT=active_dev_ports)

bfrt.pre.mgid.add(MGID=1, MULTICAST_NODE_ID=[0], MULTICAST_NODE_L1_XID_VALID=[False], MULTICAST_NODE_L1_XID=[0])

# Setup log and anti-log tables
print("Generating entries for the log and antilog tables... ", end="", flush=True)
bit_strings = gen_bit_strings()
log_masks = gen_masks()
log_keys = gen_log_keys()
log_aves = gen_log_aves()
log_vals = gen_log_vals()
exp_keys,exp_vals = gen_exp_keys_and_vals()

print("Adding entries to the log and antilog tables... ")
reg_new_rwnd = bfrt.inNetworkCC.pipe.SwitchIngress.new_rwnd
tbl_log_rwnd = bfrt.inNetworkCC.pipe.SwitchIngress.adjust_rwnd.tbl_log_rwnd
tbl_log_rtt_multiplier = bfrt.inNetworkCC.pipe.SwitchIngress.adjust_rwnd.tbl_log_rtt_multiplier
tbl_antilog_log_sum = bfrt.inNetworkCC.pipe.SwitchIngress.adjust_rwnd.tbl_antilog_log_sum

for i in zip(log_keys,log_masks,log_vals):
    tbl_log_rwnd.add_with_set_log_rwnd(base_rwnd=int(i[0],base=2), base_rwnd_mask=int(i[1],base=2), MATCH_PRIORITY=1, result=i[2])
    tbl_log_rtt_multiplier.add_with_set_log_rtt_multiplier(rtt_multiplier=int(i[0],base=2), rtt_multiplier_mask=int(i[1],base=2), MATCH_PRIORITY=1, result=i[2])
    
bfrt.batch_begin()
for i in zip(exp_keys,exp_vals):
        tbl_antilog_log_sum.add_with_set_rtt_scaled_rwnd(log_sum=i[0], result=i[1])
bfrt.batch_end()

""" Setup mirroring """
bfrt.mirror.cfg.add_with_normal(sid=1, direction='EGRESS', session_enable=True, ucast_egress_port=CPU_ETHERNET_DEV_PORT_ENP4S0F0, ucast_egress_port_valid=1, max_pkt_len=16384)


bfrt.complete_operations()

# """ Enable algo on pkts ingressing on dev port 129 """
# port_meta_tbl = bfrt.inNetworkCC.pipe.SwitchIngressParser.PORT_METADATA
# port_meta_tbl.add(ingress_port=129, apply_algo=1)

if hostname == 'hep': # for debugging 
    tbl_rtt_ws = bfrt.inNetworkCC.pipe.SwitchIngress.adjust_rwnd.fetch_rtt_mul_and_ws
    new_rwnd = bfrt.inNetworkCC.pipe.SwitchIngress.new_rwnd 
    tbl_rtt_ws.add_with_set_rtt_mul_and_ws(src_addr=0x0A010102, dst_addr=0x0A010101, src_port=5001, dst_port=56170, rtt_mul=2, ws=11)




