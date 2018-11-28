//
// Created by dell on 2018-11-28.
//
#include "../redismodule.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

#ifndef LOCK_SUCCESS
#define LOCK_SUCCESS 1
#endif

#ifndef LOCK_FAIL
#define LOCK_FAIL 1
#endif

static RedisModuleType *SLock;

struct SLock {
    mstime_t lock_time;
    unsigned long long client_id;
};
/**
 *  slock.lock key
 *  the comand lock will return result immediately , you shall cal lock in a while or give up locking
 * @param ctx
 * @param argv
 * @param argc
 * @return int
 */
int SlockLock_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    RedisModule_AutoMemory(ctx);

    if (argc != 1){
        return RedisModule_WrongArity(ctx);
    }
    RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[2], REDISMODULE_READ|REDISMODULE_WRITE);
    int keyType = RedisModule_KeyType(key);
    if(keyType != REDISMODULE_KEYTYPE_EMPTY && RedisModule_ModuleTypeGetType(key) == SLock){
        RedisModule_ReplyWithLongLong(ctx,LOCK_SUCCESS);
    } else{
        RedisModule_ReplyWithLongLong(ctx,LOCK_FAIL);
    }
    return REDISMODULE_OK;
}

/* ========================== "hellotype" type methods ======================= */

void *SLockRdbLoad(RedisModuleIO *rdb, int encver) {
    if (encver != 0) {
        /* RedisModule_Log("warning","Can't load data with version %d", encver);*/
        return NULL;
    }
    struct SLock *sl = RedisModule_Alloc(sizeof(sl));
    sl->client_id = RedisModule_LoadUnsigned(rdb);
    sl->lock_time = RedisModule_LoadSigned(rdb);
    return sl;
}

void SLockRdbSave(RedisModuleIO *rdb, void *value) {
    struct SLock *sl = value;
    RedisModule_SaveUnsigned(rdb,sl->client_id);
    RedisModule_SaveSigned(rdb, sl->lock_time);
}

void SLockAofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
    struct SLock *sl = value;
    RedisModule_EmitAOF(aof,"SLOCK.LOCK","sll",key,sl->lock_time, sl->client_id);
}

/* The goal of this function is to return the amount of memory used by
 * the HelloType value. */
size_t SLockMemUsage(const void *value) {
    const struct SLock *hto = value;
    return sizeof(*hto);
}

void SLockFree(void *value) {
    RedisModule_Free(value);
}

void SLockDigest(RedisModuleDigest *md, void *value) {
    struct SLock *sl = value;
    RedisModule_DigestAddLongLong(md,sl->client_id);
    RedisModule_DigestAddLongLong(md, sl->lock_time);
    RedisModule_DigestEndSequence(md);
}

/* This function must be present on each Redis module. It is used in order to
 * register the commands into the Redis server. */
int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    REDISMODULE_NOT_USED(argv);
    REDISMODULE_NOT_USED(argc);

    if (RedisModule_Init(ctx,"slock",1,REDISMODULE_APIVER_1)
        == REDISMODULE_ERR) return REDISMODULE_ERR;

    RedisModuleTypeMethods tm = {
            .version = REDISMODULE_TYPE_METHOD_VERSION,
            .rdb_load = SLockRdbLoad,
            .rdb_save = SLockRdbSave,
            .aof_rewrite = SLockAofRewrite,
            .mem_usage = SLockMemUsage,
            .free = SLockFree,
            .digest = SLockDigest
    };

    SLock = RedisModule_CreateDataType(ctx,"slock",0,&tm);
    if (SLock == NULL) return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"slock.lock",
                                  SlockLock_RedisCommand,"write deny-oom",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"hellotype.range",
                                  HelloTypeRange_RedisCommand,"readonly",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"hellotype.len",
                                  HelloTypeLen_RedisCommand,"readonly",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    return REDISMODULE_OK;
}
