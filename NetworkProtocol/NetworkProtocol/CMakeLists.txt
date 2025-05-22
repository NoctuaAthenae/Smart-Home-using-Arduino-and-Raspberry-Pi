cmake_minimum_required(VERSION 3.31)
project(NetworkProtocol)

set(CMAKE_CXX_STANDARD 11)

add_library(NetworkProtocol STATIC messageObjects.cpp)
