#ifndef BFRT_H
#define BFRT_H

#include <mutex>

#include <bf_types/bf_types.h>
#include <bf_rt/bf_rt_common.h>
#include <bf_rt/bf_rt_info.hpp>
#include <bf_rt/bf_rt_table.hpp>
#include <bf_rt/bf_rt_table_key.hpp>
#include <bf_rt/bf_rt_table_data.hpp>

#include "bfrt/bfrt_utils.hpp"
#include "utils/types.hpp"

/* 
    Singleton instance of BF Runtime environment
    Maintains required state and provides methods to interact with 
    the dataplane via BfRt APIs.
*/
class Bfruntime {
    private:

    bool m_isInitialized = false;
    static std::mutex _mutex;

    /* 
        BfRt Global Variables 
    */
    bf_rt_target_t dev_tgt;
    std::shared_ptr<bfrt::BfRtSession> session;
    const bfrt::BfRtInfo *bf_rt_info = nullptr; 

    /* 
        BfRt Tables/Registers Global Variables
    */
    DECLARE_BFRT_TABLE_VARS(working_copy)
    DECLARE_BFRT_TABLE_VARS(new_rwnd)
    DECLARE_BFRT_TABLE_VARS(sum_eg_deq_qdepth0)
    DECLARE_BFRT_TABLE_VARS(sum_eg_deq_qdepth1)
    DECLARE_BFRT_TABLE_VARS(pkt_count0)
    DECLARE_BFRT_TABLE_VARS(pkt_count1)

    working_copy_t currentWorkingCopy = 0;

    /* 
        Private constructor and destructor to avoid them being 
        called by clients.
    */
    Bfruntime();
    ~Bfruntime();

    /* Other private methods used internally */

    //
    void init();

    inline bool isInitialized() {return m_isInitialized;};

    // Inits the key and data objects for tables and registers
    void initBfRtTablesRegisters();
    
    // Sets working copy (of qdepth sum/count register) to specified value
    bf_status_t set_working_copy(working_copy_t newValue);

    // Swaps the working copy (of qdepth sum/count registers)
    bf_status_t update_working_copy(working_copy_t &currentWorkingCopy);

    
    public:

    /* Provides access to the unique instance of the singleton */
    static Bfruntime& getInstance();

    /* Sets specified rwnd for packets on ingress 'port' */
    bf_status_t set_rwnd(port_t port, rwnd_t newRwnd);

    /* Returns the avg qdepth observed from previous call */
    bf_status_t get_queuing_info(port_t egressPort, uint64_t &avgQdepth, working_copy_t &currentWorkingCopy);


    /* Singleton should not be clonable or assignable */
    Bfruntime(Bfruntime &other) = delete;
    void operator=(const Bfruntime &) = delete;

};



#endif
