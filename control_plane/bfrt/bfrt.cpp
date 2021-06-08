#include <bf_rt/bf_rt_common.h>
#include <bf_rt/bf_rt_init.hpp>
#include <bf_rt/bf_rt_info.hpp>
#include <bf_rt/bf_rt_session.hpp>
#include <bf_rt/bf_rt_table.hpp>
#include <bf_rt/bf_rt_table_key.hpp>
#include <bf_rt/bf_rt_table_data.hpp>

#include "bfrt/bfrt.hpp"
#include "bfrt/bfrt_utils.hpp"
#include "utils/utils.hpp"
#include "utils/types.hpp"
#include "unistd.h"

#include <fstream>
#include <iostream>

#define DEV_TGT_ALL_PIPES 0xFFFF
#define DEV_TGT_ALL_PARSERS 0xFF

#define BF_TM_CELL_SIZE_BYTES 80

auto fromHwFlag = bfrt::BfRtTable::BfRtTableGetFlag::GET_FROM_HW;

// defining the static mutex in the class
std::mutex Bfruntime::_mutex;

std::fstream outfile("result.txt");
/* 
// Register working_copy
const bfrt::BfRtTable *working_copy = nullptr;
std::unique_ptr<bfrt::BfRtTableKey> working_copy_key;
std::unique_ptr<bfrt::BfRtTableData> 
y;
bf_rt_id_t working_copy_key_id = 0;
bf_rt_id_t working_copy_data_id = 0;

// Register new_rwnd
const bfrt::BfRtTable *new_rwnd = nullptr;
std::unique_ptr<bfrt::BfRtTableKey> new_rwnd_key;
std::unique_ptr<bfrt::BfRtTableData> new_rwnd_data;
bf_rt_id_t new_rwnd_key_id = 0;
bf_rt_id_t new_rwnd_data_id = 0;

// Register sum_eg_deq_qdepth0
const bfrt::BfRtTable *sum_eg_deq_qdepth0 = nullptr;
std::unique_ptr<bfrt::BfRtTableKey> sum_eg_deq_qdepth0_key;
std::unique_ptr<bfrt::BfRtTableData> sum_eg_deq_qdepth0_data;
bf_rt_id_t sum_eg_deq_qdepth0_key_id = 0;
bf_rt_id_t sum_eg_deq_qdepth0_data_id = 0;

// Register sum_eg_deq_qdepth1
const bfrt::BfRtTable *sum_eg_deq_qdepth1 = nullptr;
std::unique_ptr<bfrt::BfRtTableKey> sum_eg_deq_qdepth1_key;
std::unique_ptr<bfrt::BfRtTableData> sum_eg_deq_qdepth1_data;
bf_rt_id_t sum_eg_deq_qdepth1_key_id = 0;
bf_rt_id_t sum_eg_deq_qdepth1_data_id = 0;

// Register pkt_count0
const bfrt::BfRtTable *pkt_count0 = nullptr;
std::unique_ptr<bfrt::BfRtTableKey> pkt_count0_key;
std::unique_ptr<bfrt::BfRtTableData> pkt_count0_data;
bf_rt_id_t pkt_count0_key_id = 0;
bf_rt_id_t pkt_count0_data_id = 0;

// Register pkt_count1
const bfrt::BfRtTable *pkt_count1 = nullptr;
std::unique_ptr<bfrt::BfRtTableKey> pkt_count1_key;
std::unique_ptr<bfrt::BfRtTableData> pkt_count1_data;
bf_rt_id_t pkt_count1_key_id = 0;
bf_rt_id_t pkt_count1_data_id = 0;

*/

Bfruntime& Bfruntime::getInstance(){
   
    // making the method thread safe for instance creation
    std::lock_guard<std::mutex> lock(_mutex);

    // will create single instance on first invocation
    static Bfruntime instance;
    if(!instance.isInitialized()){
        instance.init();
    }

    return instance;
}



Bfruntime::Bfruntime(){}
Bfruntime::~Bfruntime(){
    bf_status_t status;

    status = this->session->sessionDestroy(); CHECK_BF_STATUS(status);
    status = this->pcpp_session->sessionDestroy(); CHECK_BF_STATUS(status);

}

void Bfruntime::init(){
    bf_status_t status;

    // Init dev_tgt
    memset(&this->dev_tgt, 0, sizeof(this->dev_tgt));
    this->dev_tgt.dev_id = 0;
    this->dev_tgt.pipe_id = DEV_TGT_ALL_PIPES;

    /* Create BfRt session and retrieve BfRt Info */
    // Create a new BfRt session
    this->session = bfrt::BfRtSession::sessionCreate();
    if(this->session == nullptr){
        printf("ERROR: Couldn't create BfRtSession\n");
        exit(1); 
    }

    this->pcpp_session = bfrt::BfRtSession::sessionCreate();
    if(this->pcpp_session == nullptr){
        printf("ERROR: Couldn't create BfRtSession\n");
        exit(1); 
    }

    // Get ref to the singleton devMgr
    bfrt::BfRtDevMgr &dev_mgr = bfrt::BfRtDevMgr::getInstance(); 
    status = dev_mgr.bfRtInfoGet(this->dev_tgt.dev_id, PROG_NAME, &this->bf_rt_info);

    if(status != BF_SUCCESS){
        printf("ERROR: Could not retrieve BfRtInfo: %s\n", bf_err_str(status));
        exit(status);
    }

    printf("Retrieved BfRtInfo successfully!\n");

    // Initialize the tables and registers
    this->initBfRtTablesRegisters();

    this->m_isInitialized = true;
}

void Bfruntime::initBfRtTablesRegisters(){

    bf_status_t status;

   /*  // Register working_copy
    status = bf_rt_info->bfrtTableFromNameGet("SwitchEgressControl.working_copy", &working_copy);
    CHECK_BF_STATUS(status);

    // Index field ID
    status = working_copy->keyFieldIdGet("$REGISTER_INDEX", &working_copy_key_id);
    CHECK_BF_STATUS(status);
    // Data field ID
    status = working_copy->dataFieldIdGet("SwitchEgressControl.working_copy.f1", &working_copy_data_id);
    CHECK_BF_STATUS(status);

    status = working_copy->keyAllocate(&working_copy_key); CHECK_BF_STATUS(status);
    status = working_copy->keyReset(working_copy_key.get()); CHECK_BF_STATUS(status);

    status = working_copy->dataAllocate(&working_copy_data); CHECK_BF_STATUS(status);
    status = working_copy->dataReset(working_copy_data.get()); CHECK_BF_STATUS(status);
 */
    INIT_BFRT_REG_VARS(this->bf_rt_info, SwitchEgressControl, working_copy, status)
    INIT_BFRT_REG_VARS(this->bf_rt_info, SwitchIngress, new_rwnd, status)
    INIT_BFRT_REG_VARS(this->bf_rt_info, SwitchEgressControl, sum_eg_deq_qdepth0, status)
    INIT_BFRT_REG_VARS(this->bf_rt_info, SwitchEgressControl, sum_eg_deq_qdepth1, status)
    INIT_BFRT_REG_VARS(this->bf_rt_info, SwitchEgressControl, pkt_count0, status)
    INIT_BFRT_REG_VARS(this->bf_rt_info, SwitchEgressControl, pkt_count1, status)

    // working_copy register index is always going to be zero
    status = working_copy_key->setValue(working_copy_key_id, static_cast<uint64_t>(0));
    CHECK_BF_STATUS(status);


    /* Init table fetch_rtt_mul_and_ws */
    // Get the table
    status = bf_rt_info->bfrtTableFromNameGet("SwitchIngress.adjust_rwnd.fetch_rtt_mul_and_ws", &fetch_rtt_mul_and_ws); 
    CHECK_BF_STATUS(status);

    // Init keyField Ids
    status = fetch_rtt_mul_and_ws->keyFieldIdGet("hdr.ipv4.src_addr", &fetch_rtt_mul_and_ws_key_ipv4_src_id);
    CHECK_BF_STATUS(status);
    status = fetch_rtt_mul_and_ws->keyFieldIdGet("hdr.ipv4.dst_addr", &fetch_rtt_mul_and_ws_key_ipv4_dst_id);
    CHECK_BF_STATUS(status);
    status = fetch_rtt_mul_and_ws->keyFieldIdGet("hdr.tcp.src_port", &fetch_rtt_mul_and_ws_key_tcp_src_port_id);
    CHECK_BF_STATUS(status);
    status = fetch_rtt_mul_and_ws->keyFieldIdGet("hdr.tcp.dst_port", &fetch_rtt_mul_and_ws_key_tcp_dst_port_id);
    CHECK_BF_STATUS(status);
    // Init actionId
    status = fetch_rtt_mul_and_ws->actionIdGet("SwitchIngress.adjust_rwnd.set_rtt_mul_and_ws", &set_rtt_mul_and_ws_action_id);
    CHECK_BF_STATUS(status);
    printf("Action ID for set_rtt_mul_and_ws is %d\n", set_rtt_mul_and_ws_action_id);
    // Init dataField Ids
    status = fetch_rtt_mul_and_ws->dataFieldIdGet("rtt_mul", set_rtt_mul_and_ws_action_id, &set_rtt_mul_and_ws_action_field_rtt_mul_id);
    CHECK_BF_STATUS(status);
    printf("RTT MUL datafield ID is %u\n", set_rtt_mul_and_ws_action_field_rtt_mul_id);
    fflush(stdout);
    status = fetch_rtt_mul_and_ws->dataFieldIdGet("ws", set_rtt_mul_and_ws_action_id, &set_rtt_mul_and_ws_action_field_ws_id);
    CHECK_BF_STATUS(status);
    printf("WS datafield ID is %u\n", set_rtt_mul_and_ws_action_field_ws_id);
    fflush(stdout);
    // Allocate and reset key objects
    status = fetch_rtt_mul_and_ws->keyAllocate(&fetch_rtt_mul_and_ws_key1);
    CHECK_BF_STATUS(status);
    status = fetch_rtt_mul_and_ws->keyReset(fetch_rtt_mul_and_ws_key1.get());
    CHECK_BF_STATUS(status);
    status = fetch_rtt_mul_and_ws->keyAllocate(&fetch_rtt_mul_and_ws_key2);
    CHECK_BF_STATUS(status);
    status = fetch_rtt_mul_and_ws->keyReset(fetch_rtt_mul_and_ws_key2.get());
    CHECK_BF_STATUS(status);
    // Allocate and reset data objects
    status = fetch_rtt_mul_and_ws->dataAllocate(&fetch_rtt_mul_and_ws_data1);
    CHECK_BF_STATUS(status);
    status = fetch_rtt_mul_and_ws->dataReset(set_rtt_mul_and_ws_action_id, fetch_rtt_mul_and_ws_data1.get());
    CHECK_BF_STATUS(status);
    status = fetch_rtt_mul_and_ws->dataAllocate(&fetch_rtt_mul_and_ws_data2);
    CHECK_BF_STATUS(status);
    status = fetch_rtt_mul_and_ws->dataReset(set_rtt_mul_and_ws_action_id, fetch_rtt_mul_and_ws_data2.get());
    CHECK_BF_STATUS(status);

    // set consistent initial currentWorkingCopy in CP and DP
    this->currentWorkingCopy = 0;
    this->set_working_copy(this->currentWorkingCopy);

    printf("Initialized BfRt tables and registers successfully!\n");

}


/* Updates the working copy bit in the dataplane */
bf_status_t Bfruntime::set_working_copy(working_copy_t newValue){

    bf_status_t status;

    status = working_copy_data->setValue(working_copy_data_id, static_cast<uint64_t>(newValue));
    CHECK_BF_STATUS(status);

    // working_copy_key is already set to 0
    status = working_copy->tableEntryMod(*session, dev_tgt, *working_copy_key, *working_copy_data);

    return status;

}

// bf_status_t get_current_rwnd(port_t port){
//     bf_status_t status;

//     status = new_rwnd_key->setValue(new_rwnd_key_id, static_cast<uint64_t>(port));
//     CHECK_BF_STATUS(status);

//     status = new_rwnd->dataReset(new_rwnd_data.get());
//     CHECK_BF_STATUS(status);

//     status = new_rwnd->tableEntryGet(*session, dev_tgt, *new_rwnd_key, fromHwFlag, new_rwnd_data.get());
//     CHECK_BF_STATUS(status);

//     std::vector<uint64_t> new_rwnd_data_vector;
//     status = new_rwnd_data->getValue(new_rwnd_data_id, &new_rwnd_data_vector);
//     CHECK_BF_STATUS(status);
//     //printf("total_eg_qdepth: %lu\n", new_rwnd_data_vector[1]);
//     currentRwnd = new_rwnd_data_vector[1];

//     return status;
// }
/* Updates the rwnd in the dataplane */
bf_status_t Bfruntime::set_rwnd(port_t port, rwnd_t newRwnd){
    
    bf_status_t status;

    status = new_rwnd_key->setValue(new_rwnd_key_id, static_cast<uint64_t>(port));
    CHECK_BF_STATUS(status);

    status = new_rwnd->dataReset(new_rwnd_data.get());
    CHECK_BF_STATUS(status);

    status = new_rwnd_data->setValue(new_rwnd_data_id, static_cast<uint64_t>(newRwnd));
    CHECK_BF_STATUS(status);

    status = new_rwnd->tableEntryMod(*session, dev_tgt, *new_rwnd_key, *new_rwnd_data); 

    return status;
}

bf_status_t Bfruntime::update_working_copy(working_copy_t &currentWorkingCopy){
    bf_status_t status;

    working_copy_t newValue = (currentWorkingCopy + 1) % 2;

    status = set_working_copy(newValue);
    CHECK_BF_STATUS(status);

    currentWorkingCopy = newValue;

    return BF_SUCCESS;

}   


bf_status_t Bfruntime::get_queuing_info(port_t egressPort, uint64_t &avgQdepth, working_copy_t &currentWorkingCopy){
    bf_status_t status;
    qdepth_t total_eg_qdepth = 0;
    qdepth_t total_pkt_count = 0;

    (void) egressPort;
    working_copy_t prev_working_copy = currentWorkingCopy;
    status = update_working_copy(currentWorkingCopy); CHECK_BF_STATUS(status);

    if(prev_working_copy == 0){
        /* fetch eg enq qdepth*/
        status = sum_eg_deq_qdepth0_key->setValue(sum_eg_deq_qdepth0_key_id, static_cast<uint64_t>(egressPort));
        CHECK_BF_STATUS(status);

        status = sum_eg_deq_qdepth0->dataReset(sum_eg_deq_qdepth0_data.get());
        CHECK_BF_STATUS(status);

        status = sum_eg_deq_qdepth0->tableEntryGet(*session, dev_tgt, *sum_eg_deq_qdepth0_key, fromHwFlag, sum_eg_deq_qdepth0_data.get());
        CHECK_BF_STATUS(status);

        std::vector<uint64_t> sum_eg_deq_qdepth0_data_vector;
        status = sum_eg_deq_qdepth0_data->getValue(sum_eg_deq_qdepth0_data_id, &sum_eg_deq_qdepth0_data_vector);
        CHECK_BF_STATUS(status);
        // printf("total_eg_qdepth: %lu\n", sum_eg_deq_qdepth0_data_vector[1]);
        total_eg_qdepth = sum_eg_deq_qdepth0_data_vector[1] * BF_TM_CELL_SIZE_BYTES;
        
        /* reset eg enq qdepth*/

        status = sum_eg_deq_qdepth0->dataReset(sum_eg_deq_qdepth0_data.get());
        CHECK_BF_STATUS(status);

        status = sum_eg_deq_qdepth0_data->setValue(sum_eg_deq_qdepth0_data_id, static_cast<uint64_t>(0));
        CHECK_BF_STATUS(status);

        status = sum_eg_deq_qdepth0->tableEntryMod(*session, dev_tgt, *sum_eg_deq_qdepth0_key, *sum_eg_deq_qdepth0_data);
        
        /* fetch pkt count*/

        status = pkt_count0_key->setValue(pkt_count0_key_id, static_cast<uint64_t>(egressPort));
        CHECK_BF_STATUS(status);

        status = pkt_count0->dataReset(pkt_count0_data.get());
        CHECK_BF_STATUS(status);

        status = pkt_count0->tableEntryGet(*session, dev_tgt, *pkt_count0_key, fromHwFlag, pkt_count0_data.get());
        CHECK_BF_STATUS(status);

        std::vector<uint64_t> pkt_count0_data_vector;
        status = pkt_count0_data->getValue(pkt_count0_data_id, &pkt_count0_data_vector);
        CHECK_BF_STATUS(status);

    //    printf("total_pkt_count: %lu\n", pkt_count0_data_vector[1]);
        total_pkt_count = pkt_count0_data_vector[1]; 

        /* reset pkt count*/

        status = pkt_count0->dataReset(pkt_count0_data.get());
        CHECK_BF_STATUS(status);
        
        status = pkt_count0_data->setValue(pkt_count0_data_id, static_cast<uint64_t>(0));
        CHECK_BF_STATUS(status);

        status = pkt_count0->tableEntryMod(*session, dev_tgt, *pkt_count0_key, *pkt_count0_data);

    //    printf("Result from working copy index 0\n");

    } else {
        /* fetch eg enq qdepth*/
        status = sum_eg_deq_qdepth1_key->setValue(sum_eg_deq_qdepth1_key_id, static_cast<uint64_t>(egressPort));
        CHECK_BF_STATUS(status);

        status = sum_eg_deq_qdepth1->dataReset(sum_eg_deq_qdepth1_data.get());
        CHECK_BF_STATUS(status);

        status = sum_eg_deq_qdepth1->tableEntryGet(*session, dev_tgt, *sum_eg_deq_qdepth1_key, fromHwFlag, sum_eg_deq_qdepth1_data.get());
        CHECK_BF_STATUS(status);

        std::vector<uint64_t> sum_eg_deq_qdepth1_data_vector;
        status = sum_eg_deq_qdepth1_data->getValue(sum_eg_deq_qdepth1_data_id, &sum_eg_deq_qdepth1_data_vector);
        CHECK_BF_STATUS(status);

    //    printf("total_eg_qdepth: %lu\n", sum_eg_deq_qdepth1_data_vector[1]);
        total_eg_qdepth = sum_eg_deq_qdepth1_data_vector[1] * BF_TM_CELL_SIZE_BYTES; 

        /* reset eg enq qdepth*/

        status = sum_eg_deq_qdepth1->dataReset(sum_eg_deq_qdepth1_data.get());
        CHECK_BF_STATUS(status);

        status = sum_eg_deq_qdepth1_data->setValue(sum_eg_deq_qdepth1_data_id, static_cast<uint64_t>(0));
        CHECK_BF_STATUS(status);

        status = sum_eg_deq_qdepth1->tableEntryMod(*session, dev_tgt, *sum_eg_deq_qdepth1_key, *sum_eg_deq_qdepth1_data);

        /* fetch pkt count*/

        status = pkt_count1_key->setValue(pkt_count1_key_id, static_cast<uint64_t>(egressPort));
        CHECK_BF_STATUS(status);

        status = pkt_count1->dataReset(pkt_count1_data.get());
        CHECK_BF_STATUS(status);

        status = pkt_count1->tableEntryGet(*session, dev_tgt, *pkt_count1_key, fromHwFlag, pkt_count1_data.get());
        CHECK_BF_STATUS(status);

        std::vector<uint64_t> pkt_count1_data_vector;
        status = pkt_count1_data->getValue(pkt_count1_data_id, &pkt_count1_data_vector);
        CHECK_BF_STATUS(status);

    //    printf("total_pkt_count: %lu\n", pkt_count1_data_vector[1]);
        total_pkt_count = pkt_count1_data_vector[1];

        /* reset pkt count*/

        status = pkt_count1->dataReset(pkt_count1_data.get());
        CHECK_BF_STATUS(status);

        status = pkt_count1_data->setValue(pkt_count1_data_id, static_cast<uint64_t>(0));
        CHECK_BF_STATUS(status);

        status = pkt_count1->tableEntryMod(*session, dev_tgt, *pkt_count1_key, *pkt_count1_data);

    //    printf("Result from working copy index 1\n");
    }
    if(total_pkt_count == 0){
        avgQdepth = 0;
    } else {
        avgQdepth = total_eg_qdepth / total_pkt_count;
    }
    printf("avg_eg_qdepth_in_bytes: %lu\n", avgQdepth);
    return status;
}


bf_status_t Bfruntime::add_rtt_ws_entry_pair(const rtt_ws_entry_pair_info_t &info){

    bf_status_t status;

    // Prepare key1
    status = fetch_rtt_mul_and_ws_key1->setValue(fetch_rtt_mul_and_ws_key_ipv4_src_id, static_cast<uint64_t>(info.srcIP));
    CHECK_BF_STATUS(status);
    status = fetch_rtt_mul_and_ws_key1->setValue(fetch_rtt_mul_and_ws_key_ipv4_dst_id, static_cast<uint64_t>(info.dstIP));
    CHECK_BF_STATUS(status);
    status = fetch_rtt_mul_and_ws_key1->setValue(fetch_rtt_mul_and_ws_key_tcp_src_port_id, static_cast<uint64_t>(info.srcPort));
    CHECK_BF_STATUS(status);
    status = fetch_rtt_mul_and_ws_key1->setValue(fetch_rtt_mul_and_ws_key_tcp_dst_port_id, static_cast<uint64_t>(info.dstPort));
    CHECK_BF_STATUS(status);

    
    // Prepare data1
    status = fetch_rtt_mul_and_ws_data1->setValue(set_rtt_mul_and_ws_action_field_rtt_mul_id, static_cast<uint64_t>(info.rtt_mul));
    CHECK_BF_STATUS(status);
    status = fetch_rtt_mul_and_ws_data1->setValue(set_rtt_mul_and_ws_action_field_ws_id, static_cast<uint64_t>(info.srcWS));
    CHECK_BF_STATUS(status);


    // Prepare key2
    status = fetch_rtt_mul_and_ws_key2->setValue(fetch_rtt_mul_and_ws_key_ipv4_src_id, static_cast<uint64_t>(info.dstIP));
    CHECK_BF_STATUS(status);
    status = fetch_rtt_mul_and_ws_key2->setValue(fetch_rtt_mul_and_ws_key_ipv4_dst_id, static_cast<uint64_t>(info.srcIP));
    CHECK_BF_STATUS(status);
    status = fetch_rtt_mul_and_ws_key2->setValue(fetch_rtt_mul_and_ws_key_tcp_src_port_id, static_cast<uint64_t>(info.dstPort));
    CHECK_BF_STATUS(status);
    status = fetch_rtt_mul_and_ws_key2->setValue(fetch_rtt_mul_and_ws_key_tcp_dst_port_id, static_cast<uint64_t>(info.srcPort));
    CHECK_BF_STATUS(status);

    // Prepare data2
    status = fetch_rtt_mul_and_ws_data2->setValue(set_rtt_mul_and_ws_action_field_rtt_mul_id, static_cast<uint64_t>(info.rtt_mul));
    CHECK_BF_STATUS(status);
    status = fetch_rtt_mul_and_ws_data2->setValue(set_rtt_mul_and_ws_action_field_ws_id, static_cast<uint64_t>(info.dstWS));
    CHECK_BF_STATUS(status);

    // status = pcpp_session->beginBatch();CHECK_BF_STATUS(status);

    status = fetch_rtt_mul_and_ws->tableEntryAdd(*pcpp_session, dev_tgt, *fetch_rtt_mul_and_ws_key1, *fetch_rtt_mul_and_ws_data1);
    CHECK_BF_STATUS(status);

    status = fetch_rtt_mul_and_ws->tableEntryAdd(*pcpp_session, dev_tgt, *fetch_rtt_mul_and_ws_key2, *fetch_rtt_mul_and_ws_data2);
    CHECK_BF_STATUS(status);

    // status = pcpp_session->endBatch(false);CHECK_BF_STATUS(status);

    return status;
}
