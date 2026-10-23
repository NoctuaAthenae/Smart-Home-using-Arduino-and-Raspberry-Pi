cmake_minimum_required(VERSION 3.31)
project(NetworkProtocol)

set(CMAKE_CXX_STANDARD 11)

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