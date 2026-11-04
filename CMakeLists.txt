cmake_minimum_required(VERSION 3.22)

project(empedokles
  VERSION 0.1
  LANGUAGES CXX
)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_STRIP "")

set(CMAKE_CXX_STANDARD 20)

if(POLICY CMP0079 )
  cmake_policy(SET CMP0079 NEW)
endif()
if(POLICY CMP0135 )
  cmake_policy(SET CMP0135 NEW)
endif()

add_subdirectory(demo)
add_subdirectory(tests)
add_subdirectory(src)

include(cmake/Install.cmake)
