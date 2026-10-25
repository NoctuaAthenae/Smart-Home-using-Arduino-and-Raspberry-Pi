cmake_minimum_required(VERSION 3.31)
project(NetworkProtocol)

set(CMAKE_CXX_STANDARD 11)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(
            -fprofile-instr-generate
            -fcoverage-mapping
    )
    add_link_options(-fprofile-instr-generate)
endif()

add_library(NetworkProtocol STATIC Messages/messageObjects.cpp
        boostTests/CreateRawPackageTest.cpp
        Messages/messageBuilder.cpp
        networkDevice.cpp
        networkDevice.h
        networkHub.cpp
        networkHub.h
        timer.cpp
        timer.h)
add_subdirectory(boostTests)