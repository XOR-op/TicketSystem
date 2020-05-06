cmake_minimum_required(VERSION 3.16)
project(TicketSystem)

set(CMAKE_CXX_STANDARD 17)
aux_source_directory(src _SOURCE)
aux_source_directory(basic_component _SOURCE)
add_executable(TicketSystem ${_SOURCE} test_cpp/test.cpp src/main.cpp)

