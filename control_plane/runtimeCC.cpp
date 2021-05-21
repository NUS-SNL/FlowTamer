#include <bf_rt/bf_rt_common.h>
#include <bf_rt/bf_rt_init.hpp>
#include <bf_rt/bf_rt_info.hpp>
#include <bf_rt/bf_rt_session.hpp>
#include <bf_rt/bf_rt_table.hpp>
#include <bf_rt/bf_rt_table_key.hpp>
#include <bf_rt/bf_rt_table_data.hpp>

#include "runtimeCC.hpp"
#include "utils.hpp"
#include "types.hpp"
#include "unistd.h"

#include <fstream>
#include <iostream>
using namespace std;

#define DEV_TGT_ALL_PIPES 0xFFFF
#define DEV_TGT_ALL_PARSERS 0xFF

#define BF_TM_CELL_SIZE_BYTES 80

/* 
    BfRt Global Variables 
*/
bf_rt_target_t dev_tgt;
std::shared_ptr<bfrt::BfRtSession> session;
const bfrt::BfRtInfo *bf_rt_info = nullptr; 
auto fromHwFlag = bfrt::BfRtTable::BfRtTableGetFlag::GET_FROM_HW;

/* 
    BfRt Tables/Registers Global Variables
*/

DECLARE_BFRT_TABLE_VARS(working_copy)
DECLARE_BFRT_TABLE_VARS(new_rwnd)
DECLARE_BFRT_TABLE_VARS(sum_eg_deq_qdepth0)
DECLARE_BFRT_TABLE_VARS(sum_eg_deq_qdepth1)
DECLARE_BFRT_TABLE_VARS(pkt_count0)
DECLARE_BFRT_TABLE_VARS(pkt_count1)

/* 
    inNetworkCC State Variables with initial values
*/
working_copy_t currentWorkingCopy = 0;
port_t egressPort = 129;
uint16_t roundIntervalInMicroSec = 20000;

rwnd_t currentRwnd = 65535;
rwnd_t minimumRwnd = 118;
rwnd_t maximumRwnd = 65535;
rwnd_t rwndIncrement = 118;
rwnd_t rwndDecrement = 2;


qdepth_t lowerQdepthThreshold = 75000;
qdepth_t upperQdepthThreshold = 750000;
qdepth_t currentAvgQdepth;

fstream outfile("result.txt");
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

void initBfRt(){

    bf_status_t status;

    // Init dev_tgt
    memset(&dev_tgt, 0, sizeof(dev_tgt));
    dev_tgt.dev_id = 0;
    dev_tgt.pipe_id = DEV_TGT_ALL_PIPES;

    /* Create BfRt session and retrieve BfRt Info */
    // Create a new BfRt session
    session = bfrt::BfRtSession::sessionCreate();
    if(session == nullptr){
        printf("ERROR: Couldn't create BfRtSession\n");
        exit(1); 
    }

    // Get ref to the singleton devMgr
    bfrt::BfRtDevMgr &dev_mgr = bfrt::BfRtDevMgr::getInstance(); 
    status = dev_mgr.bfRtInfoGet(dev_tgt.dev_id, PROG_NAME, &bf_rt_info);

    if(status != BF_SUCCESS){
        printf("ERROR: Could not retrieve BfRtInfo: %s\n", bf_err_str(status));
        exit(status);
    }

    printf("Retrieved BfRtInfo successfully!\n");

}

void initBfRtTablesRegisters(){

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
    INIT_BFRT_REG_VARS(SwitchEgressControl, working_copy, status)
    INIT_BFRT_REG_VARS(SwitchIngress, new_rwnd, status)
    INIT_BFRT_REG_VARS(SwitchEgressControl, sum_eg_deq_qdepth0, status)
    INIT_BFRT_REG_VARS(SwitchEgressControl, sum_eg_deq_qdepth1, status)
    INIT_BFRT_REG_VARS(SwitchEgressControl, pkt_count0, status)
    INIT_BFRT_REG_VARS(SwitchEgressControl, pkt_count1, status)

    // working_copy register index is always going to be zero
    status = working_copy_key->setValue(working_copy_key_id, static_cast<uint64_t>(0));
    CHECK_BF_STATUS(status);

}


/* Updates the working copy bit in the dataplane */
bf_status_t set_working_copy(working_copy_t newValue){

    bf_status_t status;

    status = working_copy_data->setValue(working_copy_data_id, static_cast<uint64_t>(newValue));
    CHECK_BF_STATUS(status);

    // working_copy_key is already set to 0
    status = working_copy->tableEntryMod(*session, dev_tgt, *working_copy_key, *working_copy_data);

    return status;

}

rwnd_t min_rwnd(rwnd_t rwnd1, rwnd_t rwnd2){
    if(rwnd1 < rwnd2){
        return rwnd1;
    } else{
        return rwnd2;
    }
}

rwnd_t max_rwnd(rwnd_t rwnd1, rwnd_t rwnd2){
    if(rwnd1 > rwnd2){
        return rwnd1;
    } else{
        return rwnd2;
    }
}

bf_status_t get_current_rwnd(port_t port){
    bf_status_t status;

    status = new_rwnd_key->setValue(new_rwnd_key_id, static_cast<uint64_t>(port));
    CHECK_BF_STATUS(status);

    status = new_rwnd->dataReset(new_rwnd_data.get());
    CHECK_BF_STATUS(status);

    status = new_rwnd->tableEntryGet(*session, dev_tgt, *new_rwnd_key, fromHwFlag, new_rwnd_data.get());
    CHECK_BF_STATUS(status);

    std::vector<uint64_t> new_rwnd_data_vector;
    status = new_rwnd_data->getValue(new_rwnd_data_id, &new_rwnd_data_vector);
    CHECK_BF_STATUS(status);
    //printf("total_eg_qdepth: %lu\n", new_rwnd_data_vector[1]);
    currentRwnd = new_rwnd_data_vector[1];

    return status;
}
/* Updates the rwnd in the dataplane */
bf_status_t set_rwnd(port_t port, rwnd_t newRwnd){
    
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

bf_status_t update_working_copy(){
    bf_status_t status;

    working_copy_t newValue = (currentWorkingCopy + 1) % 2;

    status = set_working_copy(newValue);
    CHECK_BF_STATUS(status);

    currentWorkingCopy = newValue;

    return BF_SUCCESS;

}   


bf_status_t get_queuing_info(port_t egressPort, uint64_t* avgQdepth){
    bf_status_t status;
    qdepth_t total_eg_qdepth = 0;
    qdepth_t total_pkt_count = 0;

    (void) egressPort;
    working_copy_t prev_working_copy = currentWorkingCopy;
    status = update_working_copy(); CHECK_BF_STATUS(status);

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
        *avgQdepth = 0;
    } else {
        *avgQdepth = total_eg_qdepth / total_pkt_count;
    }
    printf("avg_eg_qdepth_in_bytes: %lu\n", *avgQdepth);
    return status;
}

bf_status_t inNetworkCCAlgo(){
    /* Set initial rwnd*/
    bf_status_t status;
        
    bool running = true;

    status = set_rwnd(egressPort, currentRwnd); CHECK_BF_STATUS(status);
    status = set_working_copy(currentWorkingCopy); CHECK_BF_STATUS(status);
    status = get_current_rwnd(egressPort); CHECK_BF_STATUS(status);
    usleep(roundIntervalInMicroSec);
    
    while(running){
        status = get_queuing_info(egressPort, &currentAvgQdepth);
        printf("%i\n",currentRwnd);
        if(currentAvgQdepth > upperQdepthThreshold){ // multiplicative decrement
            status = set_rwnd(egressPort, max_rwnd(minimumRwnd, currentRwnd / rwndDecrement));
            CHECK_BF_STATUS(status);
        } else if((currentAvgQdepth < lowerQdepthThreshold)){ // additive increase
            uint16_t sum = currentRwnd + rwndIncrement;
            if(sum < currentRwnd){ sum = 65535;}
            status = set_rwnd(egressPort, min_rwnd(maximumRwnd, sum)); // to avoid case where (currentRwnd + rwndIncrement > 65535
            CHECK_BF_STATUS(status);
        } else{

        }
        outfile << currentRwnd << " " << to_string(currentAvgQdepth) << endl;
        status = get_current_rwnd(egressPort); CHECK_BF_STATUS(status);
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
void inNetworkCCRuntime(){

    bf_status_t status;

    initBfRt();
    printf("\n\nFinished initBfRt");
    
    initBfRtTablesRegisters();
    printf("\n\nFinished initBfRtTablesRegisters");

    // printf("Press any key after starting iperf flows");
    // getchar();

    status = inNetworkCCAlgo();
    CHECK_BF_STATUS(status);
}