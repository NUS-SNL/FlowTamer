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

struct rtt_ws_entry_pair_info_t {
    uint32_t srcIP;
    uint32_t dstIP;
    uint16_t srcPort;
    uint16_t dstPort;
    ws_t     srcWS;
    ws_t     dstWS;
    rtt_t    rtt_mul;
};

class Bfruntime {
    private:

    bool m_isInitialized = false;
    static std::mutex _mutex;

    /* 
        BfRt Global Variables 
    */
    bf_rt_target_t dev_tgt;
    std::shared_ptr<bfrt::BfRtSession> session;
    std::shared_ptr<bfrt::BfRtSession> pcpp_session;
    const bfrt::BfRtInfo *bf_rt_info = nullptr; 

    /* 
        BfRt Tables/Registers Global Variables
    */
    DECLARE_BFRT_REG_VARS(working_copy)
    DECLARE_BFRT_REG_VARS(new_rwnd)
    DECLARE_BFRT_REG_VARS(sum_eg_deq_qdepth0)
    DECLARE_BFRT_REG_VARS(sum_eg_deq_qdepth1)
    DECLARE_BFRT_REG_VARS(pkt_count0)
    DECLARE_BFRT_REG_VARS(pkt_count1)

    // DECLARE_BFRT_TABLE_COMMON_VARS(fetch_rtt_mul_and_ws)
    const bfrt::BfRtTable *fetch_rtt_mul_and_ws = nullptr; 
    std::unique_ptr<bfrt::BfRtTableKey> fetch_rtt_mul_and_ws_key1;
    std::unique_ptr<bfrt::BfRtTableKey> fetch_rtt_mul_and_ws_key2;
    std::unique_ptr<bfrt::BfRtTableData> fetch_rtt_mul_and_ws_data1;
    std::unique_ptr<bfrt::BfRtTableData> fetch_rtt_mul_and_ws_data2;
    bf_rt_id_t fetch_rtt_mul_and_ws_key_ipv4_src_id = 0;
    bf_rt_id_t fetch_rtt_mul_and_ws_key_ipv4_dst_id = 0;
    bf_rt_id_t fetch_rtt_mul_and_ws_key_tcp_src_port_id = 0;
    bf_rt_id_t fetch_rtt_mul_and_ws_key_tcp_dst_port_id = 0;
    bf_rt_id_t set_rtt_mul_and_ws_action_id = 0;
    bf_rt_id_t set_rtt_mul_and_ws_action_field_rtt_mul_id = 0;
    bf_rt_id_t set_rtt_mul_and_ws_action_field_ws_id = 0;


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

    bf_status_t add_rtt_ws_entry_pair(const rtt_ws_entry_pair_info_t &rtt_ws_entry_pair_info);

    /* Singleton should not be clonable or assignable */
    Bfruntime(Bfruntime &other) = delete;
    void operator=(const Bfruntime &) = delete;

};



#endif
