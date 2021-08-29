cmake_minimum_required(VERSION 3.5)

project(oal LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

set(CMAKE_FIND_LIBRARY_PREFIXES "")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".dll" ".a")

find_library(EFX EFX-Util libs)
find_library(OAL OpenAL32 libs)
find_library(SND sndfile libs)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ headers/)

add_executable(oal main.cpp alhelpers.c alhelpers.h)

target_compile_options(oal PRIVATE -Wall -Wextra -pedantic)
target_link_libraries(oal LINK_PUBLIC ${OAL} ${EFX} ${SND})
