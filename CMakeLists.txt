cmake_minimum_required(VERSION 3.9)
project(osdna)

set(CMAKE_CXX_STANDARD 17)
#SET(CMAKE_C_FLAGS_DEBUG "-D_DEBUG")
SET(CMAKE_C_FLAGS_RELEASE, "-DCMAKE_BUILD_TYPE=Release")
add_executable(osdna main.cpp OSdna.h osdna_compressor.cpp osdna_compressor.h osdna_compression_core.cpp osdna_compression_core.h osdna_utils.cpp osdna_utils.h osdna_bitwriter.cpp osdna_bitwriter.h osdna_bitreader.cpp osdna_bitreader.h)