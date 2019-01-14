cmake_minimum_required(VERSION 3.10)
project(osdna)

set(CMAKE_CXX_STANDARD 17)

add_executable(osdna main.cpp OSdna.h osdna_compressor.cpp osdna_compressor.h)