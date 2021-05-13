#include <bf_rt/bf_rt_common.h>
#include <bf_rt/bf_rt_init.hpp>
#include <bf_rt/bf_rt_info.hpp>
#include <bf_rt/bf_rt_session.hpp>
#include <bf_rt/bf_rt_table.hpp>
#include <bf_rt/bf_rt_table_key.hpp>
#include <bf_rt/bf_rt_table_data.hpp>



#include "runtimeCC.hpp"
#include "utils.hpp"

#define DEV_TGT_ALL_PIPES 0xFFFF
#define DEV_TGT_ALL_PARSERS 0xFF



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
// Register working_copy
const bfrt::BfRtTable *working_copy = nullptr;
std::unique_ptr<bfrt::BfRtTableKey> working_copy_key;
std::unique_ptr<bfrt::BfRtTableData> working_copy_data;
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
    INIT_BFRT_REG_VARS(SwitchEgressControl, new_rwnd, status)
    INIT_BFRT_REG_VARS(SwitchEgressControl, sum_eg_deq_qdepth0, status)
    INIT_BFRT_REG_VARS(SwitchEgressControl, sum_eg_deq_qdepth1, status)
    INIT_BFRT_REG_VARS(SwitchEgressControl, pkt_count0, status)
    INIT_BFRT_REG_VARS(SwitchEgressControl, pkt_count1, status)

}


void inNetworkCCRuntime(){

    initBfRt();
    printf("\n\nFinished initBfRt");
    fflush(stdout);
    initBfRtTablesRegisters();
    printf("\n\nFinished initBfRtTablesRegisters");
    fflush(stdout);

}