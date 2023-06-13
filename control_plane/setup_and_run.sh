#!/bin/bash

# Load kdrv
source /home/cirlab/tools/set_sde.bash
sudo $SDE_INSTALL/bin/bf_kdrv_mod_load $SDE_INSTALL

# Setup DPDK interfaces and hugepages
sudo python3 /home/cirlab/generate-traffic/PcapPlusPlus-22.11/setup_dpdk.py restore
sudo python3 /home/cirlab/generate-traffic/PcapPlusPlus-22.11/setup_dpdk.py setup -g 512 -i enp4s0f0 enp4s0f1

# Launch the binary
LD_LIBRARY_PATH=/usr/local/lib:$SDE_INSTALL/lib:$LD_LIBRARY_PATH
sudo env "LD_LIBRARY_PATH=$LD_LIBRARY_PATH" ./inNetworkCC --thresh-high 250000 --thresh-low 25000 --no-algo
# sudo env "LD_LIBRARY_PATH=$LD_LIBRARY_PATH" ./inNetworkCC --thresh-high 250000 --thresh-low 25000
