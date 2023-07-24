cmake_minimum_required(VERSION 3.22)

project(csim C)

set(CMAKE_C_STANDARD 17)

add_executable(csim csim.c cachelab.c)