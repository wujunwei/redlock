#ifndef  SLOCK_H
#define SLOCK_H

#include "../src/redismodule.h"
#include <string.h>

static RedisModuleType *SLockType;

#define BOOL int
#define TRUE 1
#define FALSE 0

typedef struct slock {
    mstime_t lock_time;
    BOOL is_write;
    //todo read_client_list
    unsigned long long write_client_id;
} SLock;


//Error message definition
#define ERR_INVALID_EXPIRE_TIME "ERR invalid expireTime"

//COMMAND TYPE
#define COMMAND_SLOCK_UNLOCK "slock.unlock"
#define COMMAND_SLOCK_LOCK "slock.lock"
#define COMMAND_SLOCK_INFO "slock.info"
#define COMMAND_SLOCK_Rlock "slock.rlock"
#define COMMAND_SLOCK_Runlock "slock.runlock"

//MODULE INFO
#define  MODULE_NAME "slock"
#define MODULE_VERSION 2

#endif
