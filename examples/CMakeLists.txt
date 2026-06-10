cmake_minimum_required(VERSION 3.10)
project(LinkerHandSDKExamples LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4275")
    set(CMAKE_MSVC_SHOW_INCLUDES FALSE)
endif()

option(BUILD_TESTS "Build test applications" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(WIN32)
    set(LIB_SUBDIR "win64")
    set(LIB_EXTENSION ".dll")

    if(MSVC)
        set(LIB_COMPILER_SUBDIR "msvc")
        message(STATUS "Detected MSVC compiler")
    elseif(MINGW)
        set(LIB_COMPILER_SUBDIR "mingw")
        message(STATUS "Detected MinGW compiler")
    else()
        message(WARNING "Unknown compiler on Windows, defaulting to msvc")
        set(LIB_COMPILER_SUBDIR "msvc")
    endif()
elseif(UNIX AND NOT APPLE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        set(LIB_SUBDIR "arm64")
    else()
        set(LIB_SUBDIR "x86_64")
    endif()
    set(LIB_EXTENSION ".so")
elseif(APPLE)
    set(LIB_SUBDIR "x86_64")
    set(LIB_EXTENSION ".dylib")
else()
    message(WARNING "Unknown architecture, defaulting to x86_64")
    set(LIB_SUBDIR "x86_64")
    set(LIB_EXTENSION ".so")
endif()

set(LINKERHAND_LIB_NAME "linkerhand_cpp_sdk")
set(LINKERHAND_DLL_NAME "${LINKERHAND_LIB_NAME}${LIB_EXTENSION}")

# 嵌套模式：被主 CMake add_subdirectory 引入时，直接复用 SDK target，跳过 .so 路径搜索
if(TARGET linkerhand_cpp_sdk)
    set(LINKERHAND_AS_SUBPROJECT TRUE)
    set(LINKERHAND_LIB linkerhand_cpp_sdk)
    set(LINKERHAND_LIB_DIR "")
    set(LINKERHAND_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../include")
    set(LINKERHAND_DLL "$<TARGET_FILE:linkerhand_cpp_sdk>")
    message(STATUS "examples: nested under main SDK build, link target linkerhand_cpp_sdk")
endif()

if(NOT LINKERHAND_AS_SUBPROJECT)

# 独立模式：通过 find_package 解析 SDK。
# - 解压即用：examples/ 与 cmake/ 同处 release 顶层，把 release 根加进 PREFIX_PATH
# - 已安装（/usr/local 或自定义 prefix）：CMake 自动搜系统 lib/cmake/
# 也兼容用户用 -DCMAKE_PREFIX_PATH=... 或 -Dlinkerhand-cpp-sdk_DIR=... 指定。
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/..")
find_package(linkerhand-cpp-sdk 2.0 CONFIG REQUIRED)

set(LINKERHAND_LIB         LinkerHand::linkerhand_cpp_sdk)
set(LINKERHAND_INCLUDE_DIR "${linkerhand-cpp-sdk_INCLUDE_DIRS}")
set(LINKERHAND_LIB_DIR     "${linkerhand-cpp-sdk_LIBRARY_DIR}")

if(LINKERHAND_LIB_DIR)
    if(WIN32 AND EXISTS "${LINKERHAND_LIB_DIR}/${LINKERHAND_DLL_NAME}")
        set(LINKERHAND_DLL "${LINKERHAND_LIB_DIR}/${LINKERHAND_DLL_NAME}")
    elseif(UNIX AND EXISTS "${LINKERHAND_LIB_DIR}/${LINKERHAND_DLL_NAME}")
        set(LINKERHAND_DLL "${LINKERHAND_LIB_DIR}/${LINKERHAND_DLL_NAME}")
    endif()
endif()

endif() # NOT LINKERHAND_AS_SUBPROJECT

message(STATUS "Found linkerhand_cpp_sdk library: ${LINKERHAND_LIB}")
message(STATUS "Found LinkerHand headers: ${LINKERHAND_INCLUDE_DIR}")

include_directories(
    ${LINKERHAND_INCLUDE_DIR}
    ${LINKERHAND_INCLUDE_DIR}/api
    ${LINKERHAND_INCLUDE_DIR}/hand
    ${LINKERHAND_INCLUDE_DIR}/factory
    ${LINKERHAND_INCLUDE_DIR}/communication
    ${LINKERHAND_INCLUDE_DIR}/util
    ${LINKERHAND_INCLUDE_DIR}/core
)

if(WIN32)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/PCAN_Basic/Include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/libcanbus/include/windows)
else()
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/libcanbus/include/linux)
    
    # Linux: 设置 libcanbus 库路径
    set(LIBCANBUS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/libcanbus/linux")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        set(LIBCANBUS_DIR "${LIBCANBUS_DIR}/arm64")
    else()
        set(LIBCANBUS_DIR "${LIBCANBUS_DIR}/ubuntu22")
    endif()
    
    # 添加库搜索路径
    link_directories(${LIBCANBUS_DIR})
    
    # 设置运行时库路径，确保运行时能找到 libcanbus.so
    set(CMAKE_INSTALL_RPATH "${LIBCANBUS_DIR}")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
    
    message(STATUS "libcanbus directory: ${LIBCANBUS_DIR}")
endif()

# 兜底（独立消费已安装 SDK，且示例被拷出源码树）：源码树 ../third_party 不存在时，
# 改从安装位置 <prefix>/lib/linkerhand-cpp-sdk/third_party 找 CanFD/PCAN 的 third_party 头。
# CanFD.h 暴露了 ::Dev_Info（来自 Hcanbus.h），任何 include 它的下游都得有这条路径。
if(NOT LINKERHAND_AS_SUBPROJECT AND LINKERHAND_INCLUDE_DIR)
    # 安装布局 LINKERHAND_INCLUDE_DIR = <prefix>/include/linkerhand-cpp-sdk → 上溯两级得 <prefix>
    get_filename_component(_LH_PREFIX "${LINKERHAND_INCLUDE_DIR}" DIRECTORY)
    get_filename_component(_LH_PREFIX "${_LH_PREFIX}" DIRECTORY)
    set(_LH_INSTALLED_TP "${_LH_PREFIX}/lib/linkerhand-cpp-sdk/third_party")
    if(WIN32)
        if(EXISTS "${_LH_INSTALLED_TP}/PCAN_Basic/Include")
            include_directories("${_LH_INSTALLED_TP}/PCAN_Basic/Include")
        endif()
        if(EXISTS "${_LH_INSTALLED_TP}/libcanbus/include/windows")
            include_directories("${_LH_INSTALLED_TP}/libcanbus/include/windows")
        endif()
    elseif(EXISTS "${_LH_INSTALLED_TP}/libcanbus/include/linux")
        include_directories("${_LH_INSTALLED_TP}/libcanbus/include/linux")
    endif()
endif()

function(copy_dependencies target)
    if(LINKERHAND_AS_SUBPROJECT)
        # 嵌套模式：用 generator expression 取主项目 SDK target 的产物
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:linkerhand_cpp_sdk>"
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
            COMMENT "Copying linkerhand_cpp_sdk runtime to output directory"
        )
    elseif(LINKERHAND_DLL AND EXISTS "${LINKERHAND_DLL}")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${LINKERHAND_DLL}"
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
            COMMENT "Copying ${LINKERHAND_DLL_NAME} to output directory"
        )
    endif()

    if(WIN32)
        set(PCAN_DLL "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/PCAN_Basic/x64/PCANBasic.dll")
        if(EXISTS "${PCAN_DLL}")
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${PCAN_DLL}"
                    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                COMMENT "Copying PCANBasic.dll to output directory"
            )
        endif()

        set(CANFD_DLL "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/libcanbus/windows/x64/HCanbus.dll")
        if(EXISTS "${CANFD_DLL}")
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CANFD_DLL}"
                    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                COMMENT "Copying HCanbus.dll (CANFD) to output directory"
            )
        endif()
    elseif(UNIX AND NOT APPLE)
        # Linux: 复制 libcanbus.so 到输出目录
        set(CANBUS_SO "${LIBCANBUS_DIR}/libcanbus.so")
        if(EXISTS "${CANBUS_SO}")
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CANBUS_SO}"
                    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                COMMENT "Copying libcanbus.so to output directory"
            )
        endif()
        
        # Linux: 复制 libusb-1.0.so 到输出目录（libcanbus 的依赖）
        set(LIBUSB_SO "${LIBCANBUS_DIR}/libusb-1.0.so")
        if(EXISTS "${LIBUSB_SO}")
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${LIBUSB_SO}"
                    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                COMMENT "Copying libusb-1.0.so to output directory"
            )
        endif()
    endif()
endfunction()

set(COMMON_LIBS ${LINKERHAND_LIB})

if(UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
    list(APPEND COMMON_LIBS Threads::Threads)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/main_demo.cpp")
    add_executable(main_demo main_demo.cpp)
    target_link_libraries(main_demo PRIVATE ${COMMON_LIBS})
    copy_dependencies(main_demo)
endif()

if(BUILD_TESTS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/sources.cmake)

    set(EXAMPLE_STEMS ${LINKERHAND_EXAMPLES})
    if(UNIX AND NOT APPLE AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        list(APPEND EXAMPLE_STEMS ${LINKERHAND_EXAMPLES_CANFD})
    endif()

    foreach(stem ${EXAMPLE_STEMS})
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${stem}.cpp")
            get_filename_component(target "${stem}" NAME)
            string(REPLACE ".cpp" "" target "${target}")
            add_executable(${target} "${stem}.cpp")
            target_link_libraries(${target} PRIVATE ${COMMON_LIBS})
            copy_dependencies(${target})
        endif()
    endforeach()
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Target platform: ${CMAKE_SYSTEM_NAME} ${LIB_SUBDIR}")
message(STATUS "Output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
