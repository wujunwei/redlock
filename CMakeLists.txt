CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
project(slock C)


AUX_SOURCE_DIRECTORY(. SRC_LIST)
add_library(slock SHARED ${SRC_LIST})
set_target_properties(slock PROPERTIES output_name "slock")
