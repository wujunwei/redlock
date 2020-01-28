CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
project(slock C)

set(CMAKE_C_STANDARD 99)

add_library(slock SHARED
        slock.c
        )