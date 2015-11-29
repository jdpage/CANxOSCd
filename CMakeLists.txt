cmake_minimum_required (VERSION 3.1)
project (CANxOSCd)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")

add_executable (CANxOSCd
    src/daemon_main.cc
    src/config.hh
    src/config.cc
    src/msg.hh
    src/ilistener.hh
    src/listeners/stdin.hh
    src/listeners/stdin.cc
    src/listeners/stn11xx.hh
    src/listeners/stn11xx.cc
    src/translator.hh
    src/translator.cc)
set_property (TARGET CANxOSCd PROPERTY CXX_STANDARD 14)

find_package (Lua REQUIRED)
include_directories (${LUA_INCLUDE_DIRS})
target_link_libraries (CANxOSCd ${LUA_LIBRARIES})

find_package(PkgConfig REQUIRED)
pkg_check_modules (LIBLO REQUIRED liblo)
include_directories (${LIBLO_INCLUDE_DIRS})
target_link_libraries (CANxOSCd ${LIBLO_LIBRARIES})
