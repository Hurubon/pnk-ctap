cmake_minimum_required(VERSION 3.12)
project(pnk-ctap VERSION 1.1)

add_library(pnk-ctap INTERFACE)
target_include_directories(pnk-ctap INTERFACE includes)
target_include_directories(pnk-ctap INTERFACE libraries)
target_compile_features   (pnk-ctap INTERFACE cxx_std_20)
