#include <stdlib.h>
#include <stdio.h>

#include "utils.hpp"

void __check_bf_status__(bf_status_t status, const char* file, int lineNumber){
    if(status != BF_SUCCESS){
        printf("ERROR: CHECK_BF_STATUS failed at %s:%d\n", file, lineNumber);
        printf("   ==> with error: %s\n", bf_err_str(status));
        exit(status);
    }
}
