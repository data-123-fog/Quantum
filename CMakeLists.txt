cmake_minimum_required(VERSION 3.16)
project(QuantumWaveIntelligence)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    Math-Core
    Tokenizer
    System
)

add_executable(quantum
    System/main.cpp
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_link_libraries(quantum pthread)
endif()
