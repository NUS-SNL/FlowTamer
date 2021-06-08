#ifndef BFRT_UTILS_H
#define BFRT_UTILS_H

#define DECLARE_BFRT_REG_VARS(regName) \
    const bfrt::BfRtTable *regName = nullptr; \
    std::unique_ptr<bfrt::BfRtTableKey> regName##_key; \
    std::unique_ptr<bfrt::BfRtTableData> regName##_data; \
    bf_rt_id_t regName##_key_id = 0; \
    bf_rt_id_t regName##_data_id = 0;

/* #define DECLARE_BFRT_TABLE_COMMON_VARS(tableName) \
    const bfrt::BfRtTable *tableName = nullptr; \
    std::unique_ptr<bfrt::BfRtTableKey> tableName##_key; \
    std::unique_ptr<bfrt::BfRtTableData> tableName##_data; */

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
