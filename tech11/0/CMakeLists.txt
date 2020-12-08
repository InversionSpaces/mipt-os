cmake_minimum_required(VERSION 3.18)

project(SimpleFS)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FILE_OFFSET_BITS=64")

add_executable(simplefs simplefs.c)

find_package(PkgConfig REQUIRED)

pkg_check_modules(FUSE REQUIRED fuse3)

include_directories(${FUSE_INCLUDE_DIR})
target_link_libraries(simplefs ${FUSE_LIBRARIES})
