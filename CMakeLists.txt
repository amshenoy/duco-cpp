cmake_minimum_required(VERSION 3.25.0)

set(CMAKE_CXX_STANDARD 23)
# set(CMAKE_C_COMPILER "C:/Program Files/LLVM/bin/clang.exe")
# set(CMAKE_CXX_COMPILER "C:/Program Files/LLVM/bin/clang++.exe")
# set(CMAKE_GENERATOR "Unix Makefiles" CACHE INTERNAL "" FORCE)

set(CMAKE_TOOLCHAIN_FILE "C:/cppdev/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
set(VCPKG_HOST_TRIPLET "x64-windows" CACHE STRING "")

project(duco VERSION 0.1.0)

add_definitions(-DBOOST_THREAD_USE_LIB)
# add_definitions(-DBOOST_ALL_NO_LIB)
# add_definitions(-DBOOST_ALL_DYN_LINK)

find_package(Boost REQUIRED COMPONENTS
    date_time
    program_options
    iostreams
    filesystem
    # endian
)
include_directories( ${Boost_INCLUDE_DIR} )

# find_package(Boost REQUIRED COMPONENTS iostreams)
find_package(CURL CONFIG REQUIRED)
find_package(liblzma CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)

# ------------------------------------------------

# DOWNLOADER

# add_executable(download-cli)

# target_link_libraries(download-cli
#     PRIVATE
#         ${Boost_LIBRARIES}
#         CURL::libcurl
#         liblzma::liblzma
#         fmt::fmt
#         range-v3 range-v3-meta range-v3::meta range-v3-concepts
# )

# # target_include_directories(download-cli PRIVATE ${Boost_INCLUDE_DIRS})

# target_sources(download-cli
#     PRIVATE
#         download_cli.cpp
#         duco/downloader.hpp
#         duco/downloader.cpp
#         utils/decompress_range.hpp
# )

# ------------------------------------------------


# ------------------------------------------------

# SPEED TEST

add_executable(test-client)

target_link_libraries(test-client
    PRIVATE
        ${Boost_LIBRARIES}
        CURL::libcurl
        liblzma::liblzma
        fmt::fmt
        # ixwebsocket::ixwebsocket
)

target_sources(test-client
    PRIVATE
        speed_test.cpp
        # reader_test.cpp
        # utils/date_range.hpp
        utils/decompress_range.hpp
        # duco/duco.hpp
        utils/decompress.hpp
        duco/downloader.hpp
        duco/downloader.cpp
        utils/samples.hpp
        # duco/__generator.hpp
)