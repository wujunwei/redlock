#ifndef  SLOCK_H
#define SLOCK_H

#include "../src/redismodule.h"
#include <ctype.h>
#include <string.h>

static RedisModuleType *SLockType;

typedef struct slock {
    mstime_t lock_time;
    unsigned long long client_id;
} SLock;


//Error message definition
#define ERR_INVALID_EXPIRE_TIME "ERR invalid expireTime"

//COMMAND TYPE
#define COMMAND_SLOCK_UNLOCK "slock.unlock"
#define COMMAND_SLOCK_LOCK "slock.lock"
#define COMMAND_SLOCK_INFO "slock.info"


//MODULE INFO
#define  MODULE_NAME "slock"
#define MODULE_VERSION 1

#endif
