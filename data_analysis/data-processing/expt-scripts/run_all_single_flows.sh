#!/bin/bash

# Color
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

for cc in cubic bbr;do 
    for rtt in 10 50 100 200 300;do
        echo -e "${YELLOW}Running for $cc ${rtt}ms${NC}"
        ./test_single_flow.sh $cc $rtt
        sleep 2
    done
done
