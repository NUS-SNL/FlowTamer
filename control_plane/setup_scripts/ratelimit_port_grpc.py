#!/usr/bin/env python3

# Assumes valid PYTHONPATH
import bfrt_grpc.client as gc

# Connect to the BF Runtime server
for bfrt_client_id in range(10):
    try:
        interface = gc.ClientInterface(
            grpc_addr="localhost:50052",
            client_id=bfrt_client_id,
            device_id=0,
            num_tries=1,
        )
        # print("Connected to BF Runtime Server as client", bfrt_client_id)
        break
    except:
        # print("Could not connect to BF Runtime Server")
        quit

# Get information about the running program
bfrt_info = interface.bfrt_info_get()
# print("The target is running the P4 program: {}".format(bfrt_info.p4_name_get()))

# Establish that you are the "main" client
if bfrt_client_id == 0:
    interface.bind_pipeline_config(bfrt_info.p4_name_get())

dev_tgt = gc.Target(0)


def get_devport(frontpanel, lane):
    port_hdl_info = bfrt_info.table_get("$PORT_HDL_INFO")
    key = port_hdl_info.make_key(
        [gc.KeyTuple("$CONN_ID", frontpanel), gc.KeyTuple("$CHNL_ID", lane)]
    )
    for data, _ in port_hdl_info.entry_get(dev_tgt, [key], {"from_hw": False}):
        devport = data.to_dict()["$DEV_PORT"]
        if devport:
            return devport


port132 = get_devport(1, 0)
port133 = get_devport(1, 1)

import sys
rate_kbps = int(sys.argv[1])

tm_sched_shaping = bfrt_info.table_get("tf1.tm.port.sched_shaping")
tm_sched_shaping_keys = [
    tm_sched_shaping.make_key([gc.KeyTuple("dev_port", port132)]),
    tm_sched_shaping.make_key([gc.KeyTuple("dev_port", port133)]),
]
tm_sched_shaping_data = [
    tm_sched_shaping.make_data(
        [
            gc.DataTuple("unit", str_val="BPS"),
            gc.DataTuple("provisioning", str_val="UPPER"),
            gc.DataTuple("max_rate", rate_kbps),
            gc.DataTuple("max_burst_size", 0),
        ]
    ),
    tm_sched_shaping.make_data(
        [
            gc.DataTuple("unit", str_val="BPS"),
            gc.DataTuple("provisioning", str_val="UPPER"),
            gc.DataTuple("max_rate", rate_kbps),
            gc.DataTuple("max_burst_size", 0),
        ]
    ),
]
tm_sched_shaping.entry_mod(dev_tgt, tm_sched_shaping_keys, tm_sched_shaping_data)
print("Programmed Port Shaping: Rate = {}kbps".format(rate_kbps))

tm_sched_cfg = bfrt_info.table_get("tf1.tm.port.sched_cfg")
tm_sched_cfg_keys = [
    tm_sched_cfg.make_key([gc.KeyTuple("dev_port", port132)]),
    tm_sched_cfg.make_key([gc.KeyTuple("dev_port", port133)]),
]
tm_sched_cfg_data = [
    tm_sched_cfg.make_data(
        [
            gc.DataTuple("max_rate_enable", bool_val=True),
        ]
    ),
    tm_sched_cfg.make_data(
        [
            gc.DataTuple("max_rate_enable", bool_val=True),
        ]
    ),
]
tm_sched_cfg.entry_mod(dev_tgt, tm_sched_cfg_keys, tm_sched_cfg_data)
print("Enabled Port Shaping")
