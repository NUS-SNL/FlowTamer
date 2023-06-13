#!/bin/bash

sudo ip netns exec ens114f0 sudo ip addr add 10.1.1.1/24 dev ens114f0
sudo ip netns exec ens114f1 sudo ip addr add 10.1.1.2/24 dev ens114f1
sudo ip netns exec ens85f0 sudo ip addr add 10.1.1.3/24 dev ens85f0
sudo ip netns exec ens85f1 sudo ip addr add 10.1.1.4/24 dev ens85f1
