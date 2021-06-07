#ifndef UTILS_H
#define UTILS_H

#include <bf_types/bf_types.h>

void __check_bf_status__(bf_status_t status, const char* file, int lineNumber);

/* Helper function to check bf_status */
#define CHECK_BF_STATUS(status) __check_bf_status__(status, __FILE__, __LINE__)


#endif
