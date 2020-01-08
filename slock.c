
//
// Created by adam on 2018-11-28.
//
#include "../src/redismodule.h"
#include <ctype.h>
#include <string.h>




static RedisModuleType *SLockType;

struct SLock {
    mstime_t lock_time;
    unsigned long long client_id;
};

/**
 *  slock.lock lock_key expire
 *  the comand lock will return result immediately ,
 *  you shall call lock in a while or give up locking
 * @param ctx
 * @param argv
 * @param argc
 * @return int
 */
int SlockLock_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    RedisModule_AutoMemory(ctx);
    if (argc < 2){
        return RedisModule_WrongArity(ctx);
    }
    RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ|REDISMODULE_WRITE);
    int keyType = RedisModule_KeyType(key);
    if(keyType == REDISMODULE_KEYTYPE_EMPTY){
        struct SLock *lock = RedisModule_Alloc(sizeof(*lock));
        lock->client_id = RedisModule_GetClientId(ctx);
        lock->lock_time = RedisModule_Milliseconds();

        RedisModule_ModuleTypeSetValue(key, SLockType, lock);
        if(argc == 3){
            long long expire_time;
            if (RedisModule_StringToLongLong(argv[2], &expire_time) != REDISMODULE_OK){
                return RedisModule_ReplyWithError(ctx,"ERR invalid expireTime");
            }
            RedisModule_SetExpire(key, expire_time);
        }
        RedisModule_ReplyWithLongLong(ctx,1);
    } else{
        if(RedisModule_ModuleTypeGetType(key) != SLockType){
            return RedisModule_ReplyWithError(ctx,REDISMODULE_ERRORMSG_WRONGTYPE);
        }
        RedisModule_ReplyWithLongLong(ctx,0);
    }
    RedisModule_CloseKey(key);
    return REDISMODULE_OK;
}


/**
 * SLOCK.INFO  key
 *  this command will return an array with two var, first is the client id and second is the time when client require the lock.
 * @param ctx
 * @param argv
 * @param agrc
 * @return int
 */
int SlockInfo_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    RedisModule_AutoMemory(ctx);
    if (argc != 2){
        return RedisModule_WrongArity(ctx);
    }

    RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ);
    if (RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY){
        return RedisModule_ReplyWithNull(ctx);
    }
    if (RedisModule_ModuleTypeGetType(key) != SLockType){ //判断键类型
        return RedisModule_ReplyWithError(ctx,REDISMODULE_ERRORMSG_WRONGTYPE);
    } else{
        struct SLock *lock = RedisModule_ModuleTypeGetValue(key);
        RedisModule_ReplyWithArray(ctx, 2);
        RedisModule_ReplyWithLongLong(ctx, lock->client_id);
        RedisModule_ReplyWithLongLong(ctx, lock->lock_time);
    }
    RedisModule_CloseKey(key);
    return REDISMODULE_OK;
}

/**
 * Slock.unlock key
 * @param ctx
 * @param argv
 * @param argc
 * @return int
 */
int SlockunLock_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    RedisModule_AutoMemory(ctx);
    if (argc != 2){
        return RedisModule_WrongArity(ctx);
    }

    RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_WRITE|REDISMODULE_READ);
    if (RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY){
        return RedisModule_ReplyWithNull(ctx);
    }
    if (RedisModule_ModuleTypeGetType(key) != SLockType){
        return RedisModule_ReplyWithError(ctx,REDISMODULE_ERRORMSG_WRONGTYPE);
    } else{
        struct SLock *lock;
        lock = RedisModule_ModuleTypeGetValue(key);
        if(lock->client_id == RedisModule_GetClientId(ctx)){
            RedisModule_DeleteKey(key);
            RedisModule_ReplyWithLongLong(ctx, 1);
        }else{
            RedisModule_ReplyWithLongLong(ctx, 0);
        }
    }
    RedisModule_CloseKey(key);
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

    SLockType = RedisModule_CreateDataType(ctx,"slocktype",0,&tm);
    if (SLockType == NULL) return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"slock.unlock", SlockunLock_RedisCommand,"write",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"slock.lock", SlockLock_RedisCommand,"write",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx,"slock.info", SlockInfo_RedisCommand,"readonly",1,1,1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    return REDISMODULE_OK;
}
