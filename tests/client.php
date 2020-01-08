<?php


$a = new Redis();
$a->connect("127.0.0.1", "6379");
//try lock
var_dump($a->rawCommand("slock.lock", ["test", 10000]));
var_dump($a->rawCommand("slock.lock", ["test", 10000]));
//try unlock
var_dump($a->rawCommand("slock.info", ["test", 10000]));
var_dump($a->rawCommand("slock.unlock", ["test", 10000]));
$a->close();
