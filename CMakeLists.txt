cmake_minimum_required(VERSION 3.16)
project(TicketSystem)

set(CMAKE_CXX_STANDARD 17)

add_executable(TicketSystem src/structure.h src/UserManager.h src/OrderManager.h basic_component/LRUBPtree.h basic_component/PageManager.h src/global.h test_cpp/test.cpp)

