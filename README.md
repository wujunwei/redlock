[![jk](https://img.shields.io/github/stars/wujunwei/redlock?color=blue)](https://github.com/wujunwei/redlock)
[![jk](https://img.shields.io/github/watchers/wujunwei/redlock?color=green)](https://github.com/wujunwei/redlock)
[![jk](https://img.shields.io/github/forks/wujunwei/redlock?color=red)](https://github.com/wujunwei/redlock)
[![jk](https://img.shields.io/badge/support_by-adam-C70039.svg?style=flat-square)]()

# redlock
a way to achieve  distributed lock by redis module

<img src="https://upload.wikimedia.org/wikipedia/en/6/6b/Redis_Logo.svg" alt="redis" height="30" />
<h3>How to install</h3>

```
git clone https://github.com/wujunwei/redlock
cd redlock
make
//and load the module in redis cli or add it into the conf
module load redlock.so
```
<h3>How to use</h3>

* **slock.lock lock_key （expire_time）**
<br />The command will return result immediately ,you shall call lock in a while or give up locking
* **slock.unlock key**
<br />Release the write-lock, if the lock wasn't been created by the same client, it will fail with 0.
* **slock.rlock key （expire_time）**
<br />Acquire the read-lock(share lock), if the lock wasn't been created by the same client, it will fail with 0.
* **slock.runlock key**
<br />Reduce the readers count of read-lock(share lock), if the readers count equal to 0 ,it will be released.
<br />This command won't be limited by the different client. if the lock isn't created by the command "slock.rlock", it will fail with 0.
* **slock.info key**
 <br />This command will return an array with four integers, first is the client id , second is the time when lock was created , the third one is whether the lock is write-lock or not (reply with 0 present lock fail, and otherwise success) and the last one is the count of require the read lock(about the lock command ,it 's always 0).

**PLEASE NOTE:** The module  recommend to be used for stand-alone or Sharding mode,use it carefully.
