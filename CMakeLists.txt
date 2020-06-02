cmake_minimum_required(VERSION 3.10)
project(TicketSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Ofast")
aux_source_directory(src _SOURCE)
aux_source_directory(basic_component _SOURCE)
add_executable(code ${_SOURCE}  src/main.cpp basic_component/PersistentList.h src/TrainManager.cpp src/main_helper.h src/commands.cpp src/commands.h src/main_helper.cpp src/commands.h src/PendingTicketManager.h src/PendingTicketManager.cpp basic_component/SLRU.h include/debug.h)

