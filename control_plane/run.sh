#!/bin/bash

LD_LIBRARY_PATH=/usr/local/lib:$SDE_INSTALL/lib:$LD_LIBRARY_PATH
sudo env "LD_LIBRARY_PATH=$LD_LIBRARY_PATH" ./inNetworkCC --thresh-high 250000 --thresh-low 25000




