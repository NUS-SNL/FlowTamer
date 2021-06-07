#ifndef BFRT_UTILS_H
#define BFRT_UTILS_H

#define DECLARE_BFRT_TABLE_VARS(tableName) \
    const bfrt::BfRtTable *tableName = nullptr; \
    std::unique_ptr<bfrt::BfRtTableKey> tableName##_key; \
    std::unique_ptr<bfrt::BfRtTableData> tableName##_data; \
    bf_rt_id_t tableName##_key_id = 0; \
    bf_rt_id_t tableName##_data_id = 0;

#define STRINGIFY(X) #X

#define INIT_BFRT_REG_VARS(bfRtInfo, pipeControl, regName, bfrtStatus) \
    bfrtStatus = bfRtInfo->bfrtTableFromNameGet(STRINGIFY(pipeControl) "." STRINGIFY(regName), &regName);\
    CHECK_BF_STATUS(bfrtStatus);\
    bfrtStatus = regName->keyFieldIdGet("$REGISTER_INDEX", &regName##_key_id);\
    CHECK_BF_STATUS(bfrtStatus);\
    bfrtStatus = regName->dataFieldIdGet(STRINGIFY(pipeControl) "." STRINGIFY(regName) ".f1", &regName##_data_id);\
    CHECK_BF_STATUS(bfrtStatus);\
    bfrtStatus = regName->keyAllocate(&regName##_key); CHECK_BF_STATUS(bfrtStatus);\
    bfrtStatus = regName->keyReset(regName##_key.get()); CHECK_BF_STATUS(bfrtStatus);\
    bfrtStatus = regName->dataAllocate(&regName##_data); CHECK_BF_STATUS(bfrtStatus);\
    bfrtStatus = regName->dataReset(regName##_data.get()); CHECK_BF_STATUS(bfrtStatus);




#endif
