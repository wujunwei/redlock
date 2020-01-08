[![jk](https://img.shields.io/github/stars/wujunwei/redlock)](https://github.com/wujunwei/redlock)
[![jk](https://img.shields.io/github/watchers/wujunwei/redlock)](https://github.com/wujunwei/redlock)
[![jk](https://img.shields.io/github/folks/wujunwei/redlock)](https://github.com/wujunwei/redlock)
[![GitHub version](https://img.shields.io/github/release/picotera/redlock.svg?style=flat-square)](https://github.com/wujunwei/redlock/releases/latest)
[![jk](https://img.shields.io/badge/held_toghether_by-adam-C70039.svg?style=flat-square)]()

# redlock
a way to achieve  distributed lock by redis module


<h3> <img src="https://upload.wikimedia.org/wikipedia/en/6/6b/Redis_Logo.svg" alt="redis" height="30" align="top"/> How to use</h3>


* **slock.lock lock_key expire**
 the comand lock will return result immediately ,you shall call lock in a while or give up locking
* **SLOCK.INFO  key**
 this command will return an array with two var, first is the client id and second is the time when client require the lock.
* **Slock.unlock key**
release the lock, if the lock wasn't been created by the same client, it will fail with 0.


**PLEASE NOTE:** The module does *NOT* support spin lock,use it carefully.
