cmake_minimum_required(VERSION 3.10)
project(my_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 解压即用：把发布包目录加进 CMake 搜索路径，无需安装。
# 等价命令行：cmake -S . -B build -DCMAKE_PREFIX_PATH=${CMAKE_SOURCE_DIR}/linkerhand-cpp-sdk
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/linkerhand-cpp-sdk")

find_package(linkerhand-cpp-sdk 2.0 CONFIG REQUIRED)

add_executable(demo main.cpp)
target_link_libraries(demo PRIVATE LinkerHand::linkerhand_cpp_sdk)

# 运行时定位 .so：把 SDK lib 目录写进 RPATH，免去 LD_LIBRARY_PATH。
# linkerhand-cpp-sdk_LIBRARY_DIR 由 cmake/linkerhand-cpp-sdk-config.cmake 设置。
set_target_properties(demo PROPERTIES
    BUILD_RPATH   "${linkerhand-cpp-sdk_LIBRARY_DIR}"
    INSTALL_RPATH "${linkerhand-cpp-sdk_LIBRARY_DIR}"
)
