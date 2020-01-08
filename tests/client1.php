<?php


$a = new Redis();
$a->connect("127.0.0.1", "6379");
//try lock,it will fail
var_dump($a->rawCommand("slock.lock", ["test", 10000]));
//try unlock it will fail too when the lock was locked by client.php
var_dump($a->rawCommand("slock.unlock", "test"));
$a->close();
