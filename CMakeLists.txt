cmake_minimum_required(VERSION 3.12)
project(pnk-ctap VERSION 1.0)

add_library(pnk-ctap INTERFACE)
target_include_directories(pnk-ctap INTERFACE inc)
target_include_directories(pnk-ctap INTERFACE lib)
target_compile_features   (pnk-ctap INTERFACE cxx_std_20)
