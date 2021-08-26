#include <iostream>
#include <unistd.h>

#include "algo/algo.hpp"
#include "utils/types.hpp"
#include "bfrt/bfrt.hpp"

/* 
    inNetworkCC State Variables with initial values
*/
working_copy_t currentWorkingCopy = 0;
port_t egressPort = 129;
uint16_t roundIntervalInMicroSec = 20000;

rwnd_t minimumRwnd = 118;
rwnd_t maximumRwnd = 1751122; // 50 MB for RTT of 300ms ==> 50/30 = 1.67MB for RTT of 10ms
rwnd_t currentRwnd = maximumRwnd;
rwnd_t rwndIncrement = 1500;
rwnd_t rwndDecrement = 2;


qdepth_t currentAvgQdepth;


bf_status_t inNetworkCCAlgo(std::fstream &outfile, bool &algo_running, const algo_params_t& algo_params){
    
    bf_status_t status;

    Bfruntime& bfrt = Bfruntime::getInstance();

    qdepth_t lowerQdepthThreshold = algo_params.thresh_low; // 75000;
    qdepth_t upperQdepthThreshold = algo_params.thresh_high; // 750000;

    if(algo_params.no_algo){
        // Set the initial rwnd to zero
        status = bfrt.set_rwnd(egressPort, 0); CHECK_BF_STATUS(status);
        printf("Not running any algo...\n");
    }
    else{
        /* Set initial rwnd and working copy */
        status = bfrt.set_rwnd(egressPort, currentRwnd); CHECK_BF_STATUS(status);
        // status = set_working_copy(currentWorkingCopy); CHECK_BF_STATUS(status);
    }
    usleep(roundIntervalInMicroSec);
    
    while(algo_running){
        if(!algo_params.no_algo){
            status = bfrt.get_queuing_info(egressPort, currentAvgQdepth, currentWorkingCopy);
            printf("%i\n",currentRwnd);
            if(currentAvgQdepth > upperQdepthThreshold){ // multiplicative decrement
                currentRwnd = max_rwnd(minimumRwnd, currentRwnd / rwndDecrement);
                status = bfrt.set_rwnd(egressPort, currentRwnd);
                CHECK_BF_STATUS(status);
            } else if((currentAvgQdepth < lowerQdepthThreshold)){ // additive increase
                rwnd_t sum = currentRwnd + rwndIncrement;
                if(sum < currentRwnd){ sum = maximumRwnd;} // to handle wrap around (rarely possible?)
                currentRwnd = min_rwnd(maximumRwnd, sum);
                status = bfrt.set_rwnd(egressPort, currentRwnd); // to avoid case where (currentRwnd + rwndIncrement > 65535
                CHECK_BF_STATUS(status);
            } else{

            }
            outfile << currentRwnd << " " << std::to_string(currentAvgQdepth) << std::endl;
        }
        usleep(roundIntervalInMicroSec);
    }
    outfile.close();
    // Step 1: init the working_copy to 0, initial rwnd
    // Step 2: Start the infinite loop for each round (interval)
    // In each round:
    // - Input: update working copy, read old copy, reset to zero and compute avg qdepth
    // - Algo: call the algo function with new avg qdepth. Returns new_rwnd
    // - Output: set the new rwnd in the data plane register
    return status;
}
