#ifndef ALGO_H
#define ALGO_H

#include "utils/utils.hpp"
#include "utils/types.hpp"
#include <fstream>

bf_status_t inNetworkCCAlgo(std::fstream &outfile, bool &algo_running);



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
