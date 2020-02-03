//
// Created by adam on 2018-11-28.
//
#include "slock.h"

/**
 *  slock.lock lock_key expire
 *  the command lock will return result immediately ,
 *  you shall call lock in a while or give up locking
 * @param ctx
 * @param argv
 * @param argc
 * @return int
 */
SLock *createSlock(RedisModuleCtx *ctx, int is_write) {
    SLock *lock = RedisModule_Alloc(sizeof(*lock));
    lock->write_client_id = RedisModule_GetClientId(ctx);
    lock->reader_count = 0;
    lock->lock_time = RedisModule_Milliseconds();
    lock->is_write = is_write ? TRUE : FALSE;
    return lock;
}

int SlockLock_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    RedisModule_AutoMemory(ctx);
    if (argc < 2) {
        return RedisModule_WrongArity(ctx);
    }
    RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int keyType = RedisModule_KeyType(key);
    if (keyType == REDISMODULE_KEYTYPE_EMPTY) {
        SLock *lock = createSlock(ctx, TRUE);
        RedisModule_ModuleTypeSetValue(key, SLockType, lock);
        if (argc == 3) {
            long long expire_time;
            if (RedisModule_StringToLongLong(argv[2], &expire_time) != REDISMODULE_OK) {
                return RedisModule_ReplyWithError(ctx, ERR_INVALID_EXPIRE_TIME);
            }
            RedisModule_SetExpire(key, expire_time);
        }
        RedisModule_ReplyWithLongLong(ctx, 1);
    } else {
        if (RedisModule_ModuleTypeGetType(key) != SLockType) {
            return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        }
        RedisModule_ReplyWithLongLong(ctx, 0);
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
int SlockUnLock_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    RedisModule_AutoMemory(ctx);
    if (argc != 2) {
        return RedisModule_WrongArity(ctx);
    }

    RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_WRITE | REDISMODULE_READ);
    if (RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY) {
        return RedisModule_ReplyWithNull(ctx);
    }
    if (RedisModule_ModuleTypeGetType(key) != SLockType) {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    } else {
        SLock *lock = RedisModule_ModuleTypeGetValue(key);
        if (lock->write_client_id == RedisModule_GetClientId(ctx) && lock->is_write) {
            RedisModule_DeleteKey(key);
            RedisModule_ReplyWithLongLong(ctx, 1);
        } else {
            RedisModule_ReplyWithLongLong(ctx, 0);
        }
    }
    RedisModule_CloseKey(key);
    return REDISMODULE_OK;
}


/**
 *  slock.rlock lock_key expire
 *  the command lock will return result immediately ,
 *  you shall call lock in a while or give up locking
 * @param ctx
 * @param argv
 * @param argc
 * @return int
 */
int SlockRLock_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    RedisModule_AutoMemory(ctx);
    if (argc < 2) {
        return RedisModule_WrongArity(ctx);
    }
    RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int keyType = RedisModule_KeyType(key);
    if (keyType == REDISMODULE_KEYTYPE_EMPTY) {
        SLock *lock = createSlock(ctx, FALSE);
        RedisModule_ModuleTypeSetValue(key, SLockType, lock);
    } else {
        // Exclude write-lock
        if (RedisModule_ModuleTypeGetType(key) != SLockType) {
            return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
        }
        SLock *lock = RedisModule_ModuleTypeGetValue(key);
        if (lock->is_write) {
            return RedisModule_ReplyWithLongLong(ctx, 0);
        }
        lock->reader_count += 1;
    }
    if (argc == 3) {
        long long expire_time;
        if (RedisModule_StringToLongLong(argv[2], &expire_time) != REDISMODULE_OK) {
            return RedisModule_ReplyWithError(ctx, ERR_INVALID_EXPIRE_TIME);
        }
        RedisModule_SetExpire(key, expire_time);
    }
    RedisModule_ReplyWithLongLong(ctx, 1);
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
int SlockRUnLock_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    RedisModule_AutoMemory(ctx);
    if (argc != 2) {
        return RedisModule_WrongArity(ctx);
    }

    RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_WRITE | REDISMODULE_READ);
    if (RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY) {
        return RedisModule_ReplyWithNull(ctx);
    }
    if (RedisModule_ModuleTypeGetType(key) != SLockType) {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    SLock *lock;
    lock = RedisModule_ModuleTypeGetValue(key);
    if (!lock->is_write) {
        if (--lock->reader_count <= 0) {
            RedisModule_DeleteKey(key);
        }
        RedisModule_ReplyWithLongLong(ctx, 1);
    } else {
        RedisModule_ReplyWithLongLong(ctx, 0);
    }
    RedisModule_CloseKey(key);
    return REDISMODULE_OK;
}

/**
 * SLOCK.INFO  key
 *  this command will return an array with four integers, first is the client id , second is the time when lock was created , the third one is whether the lock is write-lock or not (reply with 0 present lock fail, and otherwise success) and the last one is the count of require the read lock(about the lock command ,it 's always 0).
 * @param ctx
 * @param argv
 * @param agrc
 * @return int
 */
int SlockInfo_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    RedisModule_AutoMemory(ctx);
    if (argc != 2) {
        return RedisModule_WrongArity(ctx);
    }

    RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ);
    if (RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY) {
        return RedisModule_ReplyWithNull(ctx);
    }
    //判断键类型
    if (RedisModule_ModuleTypeGetType(key) != SLockType) {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    } else {
        SLock *lock = RedisModule_ModuleTypeGetValue(key);
        RedisModule_ReplyWithArray(ctx, 4);
        RedisModule_ReplyWithLongLong(ctx, lock->write_client_id);
        RedisModule_ReplyWithLongLong(ctx, lock->lock_time);
        RedisModule_ReplyWithLongLong(ctx, lock->is_write);
        RedisModule_ReplyWithLongLong(ctx, lock->reader_count);
    }
    RedisModule_CloseKey(key);
    return REDISMODULE_OK;
}

/* ========================== type methods ======================= */

void *SLockRdbLoad(RedisModuleIO *rdb, int encver) {
    if (encver != 0) {
        /* RedisModule_Log("warning","Can't load data with version %d", encver);*/
        return NULL;
    }
    SLock *sl = RedisModule_Alloc(sizeof(sl));
    sl->write_client_id = RedisModule_LoadUnsigned(rdb);
    sl->lock_time = RedisModule_LoadSigned(rdb);
    sl->is_write = RedisModule_LoadSigned(rdb);
    sl->reader_count = RedisModule_LoadUnsigned(rdb);
    return sl;
}

void SLockRdbSave(RedisModuleIO *rdb, void *value) {
    SLock *sl = value;
    RedisModule_SaveUnsigned(rdb, sl->write_client_id);
    RedisModule_SaveSigned(rdb, sl->lock_time);
    RedisModule_SaveSigned(rdb, sl->is_write);
    RedisModule_SaveUnsigned(rdb, sl->reader_count);
}

void SLockAofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
    SLock *sl = value;
    if (sl->is_write) {
        RedisModule_EmitAOF(aof, "SLOCK.LOCK", "s", key);
        return;
    }

    unsigned long long count = sl->reader_count;
    while (count--){
        RedisModule_EmitAOF(aof, "SLOCK.rLOCK", "s", key);
    }

}

/* The goal of this function is to return the amount of memory used by
 * the HelloType value. */
size_t SLockMemUsage(const void *value) {
    const SLock *hto = value;
    return sizeof(*hto);
}

void SLockFree(void *value) {
    RedisModule_Free(value);
}

void SLockDigest(RedisModuleDigest *md, void *value) {
    SLock *sl = value;
    RedisModule_DigestAddLongLong(md, sl->write_client_id);
    RedisModule_DigestAddLongLong(md, sl->is_write);
    RedisModule_DigestAddLongLong(md, sl->reader_count);
    RedisModule_DigestAddLongLong(md, sl->lock_time);
    RedisModule_DigestEndSequence(md);
}

/* This function must be present on each Redis module. It is used in order to
 * register the commands into the Redis server. */
int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    REDISMODULE_NOT_USED(argv);
    REDISMODULE_NOT_USED(argc);

    if (RedisModule_Init(ctx, MODULE_NAME, MODULE_VERSION, REDISMODULE_APIVER_1)
        == REDISMODULE_ERR)
        return REDISMODULE_ERR;
    RedisModuleTypeMethods tm = {
            .version = REDISMODULE_TYPE_METHOD_VERSION,
            .rdb_load = SLockRdbLoad,
            .rdb_save = SLockRdbSave,
            .aof_rewrite = SLockAofRewrite,
            .mem_usage = SLockMemUsage,
            .free = SLockFree,
            .digest = SLockDigest
    };

    SLockType = RedisModule_CreateDataType(ctx, "slocktype", 0, &tm);
    if (SLockType == NULL) return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, COMMAND_SLOCK_UNLOCK, SlockUnLock_RedisCommand, "write", 1, 1, 1) ==
        REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, COMMAND_SLOCK_LOCK, SlockLock_RedisCommand, "write", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;


    if (RedisModule_CreateCommand(ctx, COMMAND_SLOCK_RLOCK, SlockRLock_RedisCommand, "write", 1, 1, 1) ==
        REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, COMMAND_SLOCK_RUNLOCK, SlockRUnLock_RedisCommand, "write", 1, 1, 1) ==
        REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, COMMAND_SLOCK_INFO, SlockInfo_RedisCommand, "readonly", 1, 1, 1) ==
        REDISMODULE_ERR)
        return REDISMODULE_ERR;

    return REDISMODULE_OK;
}
