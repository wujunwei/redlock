<?php


$a = new Redis();
$a->connect("127.0.0.1", "6379");
//try lock,it will fail
var_dump($a->rawCommand("slock.rlock", ["test", 10000]));
sleep(5);
var_dump($a->rawCommand("slock.rlock", ["test", 10000]));
var_dump($a->ttl("slock.rlock"));
// the expired time will be refresh.
var_dump($a->rawCommand("slock.unlock", "test"));
$a->close();
