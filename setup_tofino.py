import socket

hostname = socket.gethostname()

# Configure front-panel ports
fp_ports = []

if hostname == 'tofino1a':
    fp_ports = [1]
elif hostname == 'tofino1b':
    fp_ports = [31]
elif hostname == 'tofino1c':
    fp_ports = [31]

for fp_port in fp_ports:
    for lane in range(4):
        dp = bfrt.port.port_hdl_info.get(conn_id=fp_port, chnl_id=lane, print_ents=False).data[b'$DEV_PORT']
        bfrt.port.port.add(dev_port=dp, speed='BF_SPEED_10G', fec='BF_FEC_TYP_NONE', auto_negotiation='PM_AN_FORCE_DISABLE', port_enable=True)

# Add entries to the l2_forward table
l2_forward = bfrt.inNetworkCC.pipe.SwitchIngress.l2_forward
if hostname == 'tofino1b':
    l2_forward.add_with_forward(dst_addr=0x6cb3115309b0, port=128)
    l2_forward.add_with_forward(dst_addr=0x6cb3115309b2, port=129)
    l2_forward.add_with_forward(dst_addr=0x6cb31153099c, port=130)
    l2_forward.add_with_forward(dst_addr=0x6cb31153099e, port=131)
elif hostname == 'tofino1c':
    l2_forward.add_with_forward(dst_addr=0x3cfdfebce1c0, port=128)
    l2_forward.add_with_forward(dst_addr=0x3cfdfebce1c1, port=129)


# Setup ARP broadcast for the active dev ports
active_dev_ports = [128, 129, 130, 131]
bfrt.pre.node.add(multicast_node_id=0, multicast_rid=0, multicast_lag_id=[], dev_port=active_dev_ports)

bfrt.pre.mgid.add(mgid=1, multicast_node_id=[0], multicast_node_l1_xid_valid=[False], multicast_node_l1_xid=[0])

bfrt.complete_operations()
