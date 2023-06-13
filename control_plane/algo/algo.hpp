#ifndef ALGO_H
#define ALGO_H

#include "utils/utils.hpp"
#include "utils/types.hpp"
#include <fstream>


struct algo_params_t {
    bool no_algo;
    qdepth_t thresh_high;
    qdepth_t thresh_low;
    uint16_t round_interval_ms;

    // default constructor to set the default values
    algo_params_t():
    no_algo(false),
    thresh_high(750000),
    thresh_low(75000), // 50 packets
    round_interval_ms(20000)
    {}
};

bf_status_t inNetworkCCAlgo(std::fstream &outfile, bool &algo_running, const algo_params_t& no_algo);

inline rwnd_t min_rwnd(rwnd_t rwnd1, rwnd_t rwnd2){
    if(rwnd1 < rwnd2){
        return rwnd1;
    } else{
        return rwnd2;
    }
}

inline rwnd_t max_rwnd(rwnd_t rwnd1, rwnd_t rwnd2){
    if(rwnd1 > rwnd2){
        return rwnd1;
    } else{
        return rwnd2;
    }
}


#endif
