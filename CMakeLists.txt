cmake_minimum_required(VERSION 3.10)

project(mmuduo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread")

include_directories(include)

aux_source_directory(./src SRC)

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
add_library(mmuduo SHARED ${SRC})


add_executable(main src/main.cpp)

link_directories(${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(main mmuduo pthread)
