cmake_minimum_required(VERSION 3.25.0)
set(CMAKE_CXX_STANDARD 23)
# set(CMAKE_C_COMPILER clang)
# set(CMAKE_CXX_COMPILER clang)

set(CMAKE_TOOLCHAIN_FILE "C:/cppdev/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
set(VCPKG_HOST_TRIPLET "x64-windows" CACHE STRING "")

project(duco VERSION 0.1.0)

add_definitions(-DBOOST_THREAD_USE_LIB)
find_package(Boost REQUIRED COMPONENTS
    date_time
    program_options
    iostreams
    filesystem
    # endian
)
include_directories( ${Boost_INCLUDE_DIR} )

find_package(CURL CONFIG REQUIRED)
find_package(liblzma CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

# ------------------------------------------------

# DOWNLOADER

add_executable(download-cli)

target_link_libraries(download-cli
    PRIVATE
        ${Boost_LIBRARIES}
        CURL::libcurl
        liblzma::liblzma
        fmt::fmt
)

target_sources(download-cli
    PRIVATE
        download_cli.cpp
        duco/downloader.hpp
        duco/downloader.cpp
)

# ------------------------------------------------
