cmake_minimum_required(VERSION 3.12)

project(test)


set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../extra)
find_package(NanopbJson REQUIRED)

nanopbjson_generate_cpp(TARGET pbjson test_json.proto simple.proto)

add_executable(test 
    test2.cpp 
)

target_link_libraries(test pbjson)
