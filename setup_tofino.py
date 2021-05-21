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
        dp = bfrt.port.port_hdl_info.get(CONN_ID=fp_port, CHNL_ID=lane, print_ents=False).data[b'$DEV_PORT']
        bfrt.port.port.add(DEV_PORT=dp, SPEED='BF_SPEED_10G', FEC='BF_FEC_TYP_NONE', AUTO_NEGOTIATION='PM_AN_FORCE_DISABLE', PORT_ENABLE=True)

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
bfrt.pre.node.add(MULTICAST_NODE_ID=0, MULTICAST_RID=0, MULTICAST_LAG_ID=[], DEV_PORT=active_dev_ports)

bfrt.pre.mgid.add(MGID=1, MULTICAST_NODE_ID=[0], MULTICAST_NODE_L1_XID_VALID=[False], MULTICAST_NODE_L1_XID=[0])

bfrt.complete_operations()
