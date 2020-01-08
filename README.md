[![jk](https://img.shields.io/github/stars/wujunwei/redlock?color=blue)](https://github.com/wujunwei/redlock)
[![jk](https://img.shields.io/github/watchers/wujunwei/redlock?color=green)](https://github.com/wujunwei/redlock)
[![jk](https://img.shields.io/github/forks/wujunwei/redlock?color=red)](https://github.com/wujunwei/redlock)
[![jk](https://img.shields.io/badge/support_by-adam-C70039.svg?style=flat-square)]()

# redlock
a way to achieve  distributed lock by redis module

<img src="https://upload.wikimedia.org/wikipedia/en/6/6b/Redis_Logo.svg" alt="redis" height="30" />
<h3>  How to install</h3>

```
git clone https://github.com/wujunwei/redlock
cd redlock
make
//and load the module in redis cli or add it into the conf
module load redlock.so
```
<h3>  How to use</h3>

* **slock.lock lock_key expire**
 the comand lock will return result immediately ,you shall call lock in a while or give up locking
* **SLOCK.INFO  key**
 this command will return an array with two var, first is the client id and second is the time when client require the lock.
* **Slock.unlock key**
release the lock, if the lock wasn't been created by the same client, it will fail with 0.


**PLEASE NOTE:** The module does *NOT* support spin lock,use it carefully.
