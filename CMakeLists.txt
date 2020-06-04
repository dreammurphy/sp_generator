cmake_minimum_required(VERSION 3.16)
project(SNN)

set(CMAKE_CXX_STANDARD 11)

include_directories(nn_model)
include_directories(utils)

file(GLOB SRC_FILES "nn_model/*.cpp" "nn_model/*.h"
        "utils/*.cpp" "utils/*.h")

add_executable(SNN
        ${SRC_FILES}
        main.cpp)
