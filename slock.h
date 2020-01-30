#ifndef  SLOCK_H
#define SLOCK_H

#include "../src/redismodule.h"
#include <string.h>

static RedisModuleType *SLockType;

#define TRUE 1
#define FALSE 0

typedef struct slock {
    mstime_t lock_time;
    int is_write;
    unsigned long long reader_count;
    unsigned long long write_client_id;
} SLock;


//Error message definition
#define ERR_INVALID_EXPIRE_TIME "ERR invalid expireTime"

//COMMAND TYPE
#define COMMAND_SLOCK_UNLOCK "slock.unlock"
#define COMMAND_SLOCK_LOCK "slock.lock"
#define COMMAND_SLOCK_INFO "slock.info"
#define COMMAND_SLOCK_RLOCK "slock.rlock"
#define COMMAND_SLOCK_RUNLOCK "slock.runlock"

//MODULE INFO
#define  MODULE_NAME "slock"
#define MODULE_VERSION 2

#endif
