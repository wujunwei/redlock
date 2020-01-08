cmake_minimum_required(VERSION 3.15)
project(redlock C)

set(CMAKE_C_STANDARD 11)
#  change to your gcc path
set(CMAKE_C_COMPILER   "/usr/local/Cellar/gcc/8.1.0/bin/gcc-8")

include_directories(../src/redismodule.h)
add_executable(redlock
        slock.c)
add_custom_target(makefile COMMAND make -C  slock.c)