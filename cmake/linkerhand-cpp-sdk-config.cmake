# linkerhand-cpp-sdk-config.cmake
# 同时支持两种使用方式：
#
#   A) 安装后 find_package — 配置文件位于 <prefix>/lib/cmake/linkerhand-cpp-sdk/
#      头：<prefix>/include/linkerhand-cpp-sdk/{api,core,communication}/
#      库：<prefix>/lib/linkerhand-cpp-sdk/lib/<预编译库>
#
#   B) 解压即用 find_package — 配置文件位于 <release>/cmake/
#      头：<release>/include/{api,core,communication}/
#      库：<release>/lib/<linux|win64>/<arch>/<预编译库>
#
# 下游通用写法：
#   find_package(linkerhand-cpp-sdk CONFIG REQUIRED)
#   target_link_libraries(<tgt> PRIVATE LinkerHand::linkerhand_cpp_sdk)
#
# 解压即用时，让 CMake 找到本文件的方式（任选其一）：
#   list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/linkerhand-cpp-sdk")
#   # 或命令行：cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/linkerhand-cpp-sdk
#   # 或命令行：cmake -S . -B build -Dlinkerhand-cpp-sdk_DIR=/path/to/linkerhand-cpp-sdk/cmake

get_filename_component(_lh_self_dir "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)

# --- 布局识别 ---
# 解压布局：<release>/cmake/<this>，向上 1 级 = <release>，含 include/api/...
get_filename_component(_lh_extract_root "${_lh_self_dir}/.." ABSOLUTE)
# 安装布局：<prefix>/lib/cmake/linkerhand-cpp-sdk/<this>，向上 3 级 = <prefix>
get_filename_component(_lh_install_prefix "${_lh_self_dir}/../../.." ABSOLUTE)

set(_lh_inc "")
set(_lh_lib_search "")
set(_lh_thirdparty_root "")

if(EXISTS "${_lh_extract_root}/include/api/LinkerHandApi.h")
    # 解压即用：lib 按平台/架构子目录布局
    set(_lh_inc "${_lh_extract_root}/include")
    if(EXISTS "${_lh_extract_root}/third_party")
        set(_lh_thirdparty_root "${_lh_extract_root}/third_party")
    endif()
    if(WIN32)
        if(MSVC OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
            list(APPEND _lh_lib_search "${_lh_extract_root}/lib/win64/msvc")
        else()
            list(APPEND _lh_lib_search "${_lh_extract_root}/lib/win64/mingw")
        endif()
    elseif(UNIX AND NOT APPLE)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
            list(APPEND _lh_lib_search "${_lh_extract_root}/lib/linux/x86_64")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$")
            list(APPEND _lh_lib_search "${_lh_extract_root}/lib/linux/arm64")
        endif()
    endif()
elseif(EXISTS "${_lh_install_prefix}/include/linkerhand-cpp-sdk/api/LinkerHandApi.h")
    # 已安装：lib 平铺在 <prefix>/lib/linkerhand-cpp-sdk/lib/，third_party 平级
    set(_lh_inc "${_lh_install_prefix}/include/linkerhand-cpp-sdk")
    list(APPEND _lh_lib_search "${_lh_install_prefix}/lib/linkerhand-cpp-sdk/lib")
    if(EXISTS "${_lh_install_prefix}/lib/linkerhand-cpp-sdk/third_party")
        set(_lh_thirdparty_root "${_lh_install_prefix}/lib/linkerhand-cpp-sdk/third_party")
    endif()
endif()

if(NOT _lh_inc)
    set(linkerhand-cpp-sdk_FOUND FALSE)
    set(linkerhand-cpp-sdk_NOT_FOUND_MESSAGE
        "linkerhand-cpp-sdk: 既不在解压布局 ${_lh_extract_root} 也不在安装布局 ${_lh_install_prefix} 下找到 include/")
    return()
endif()

include(CMakeFindDependencyMacro)
find_dependency(Threads)

# --- 选库：Linux .so / Windows .dll(+import lib) ---
set(_lh_imported_loc "")
set(_lh_imported_implib "")
set(_lh_lib_dir "")

foreach(_dir IN LISTS _lh_lib_search)
    if(NOT _lh_imported_loc AND IS_DIRECTORY "${_dir}")
        if(WIN32)
            if(EXISTS "${_dir}/linkerhand_cpp_sdk.dll")
                set(_lh_imported_loc "${_dir}/linkerhand_cpp_sdk.dll")
                set(_lh_lib_dir "${_dir}")
            endif()
        elseif(UNIX)
            if(EXISTS "${_dir}/linkerhand_cpp_sdk.so")
                set(_lh_imported_loc "${_dir}/linkerhand_cpp_sdk.so")
                set(_lh_lib_dir "${_dir}")
            endif()
        endif()
    endif()
endforeach()

if(WIN32 AND _lh_lib_dir)
    if(MSVC OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
        if(EXISTS "${_lh_lib_dir}/linkerhand_cpp_sdk.lib")
            set(_lh_imported_implib "${_lh_lib_dir}/linkerhand_cpp_sdk.lib")
        endif()
    else()
        if(EXISTS "${_lh_lib_dir}/linkerhand_cpp_sdk.a")
            set(_lh_imported_implib "${_lh_lib_dir}/linkerhand_cpp_sdk.a")
        elseif(EXISTS "${_lh_lib_dir}/linkerhand_cpp_sdk.dll")
            set(_lh_imported_implib "${_lh_lib_dir}/linkerhand_cpp_sdk.dll")
        endif()
    endif()
endif()

if(NOT _lh_imported_loc)
    set(linkerhand-cpp-sdk_FOUND FALSE)
    set(linkerhand-cpp-sdk_NOT_FOUND_MESSAGE
        "linkerhand-cpp-sdk: 候选 lib 目录均无匹配文件: ${_lh_lib_search}")
    return()
endif()

if(NOT TARGET LinkerHand::linkerhand_cpp_sdk)
    add_library(LinkerHand::linkerhand_cpp_sdk SHARED IMPORTED)
    set_target_properties(LinkerHand::linkerhand_cpp_sdk PROPERTIES
        IMPORTED_LOCATION "${_lh_imported_loc}"
        # 顶层 + 三个子目录都暴露，下游既能写 #include "api/LinkerHandApi.h"
        # 也能写扁平的 #include "LinkerHandApi.h"
        INTERFACE_INCLUDE_DIRECTORIES "${_lh_inc};${_lh_inc}/api;${_lh_inc}/core;${_lh_inc}/communication"
        INTERFACE_LINK_LIBRARIES "Threads::Threads"
    )
    if(_lh_imported_implib)
        set_target_properties(LinkerHand::linkerhand_cpp_sdk PROPERTIES
            IMPORTED_IMPLIB "${_lh_imported_implib}")
    endif()
    # 公共头 CanFD.h / PCANBus.h / CanFrame.h 引用了 third_party 头（Hcanbus.h /
    # PCANBasic.h）。把对应 include 目录补到 imported target，下游 link 后无需手动配。
    if(_lh_thirdparty_root)
        if(WIN32)
            if(EXISTS "${_lh_thirdparty_root}/libcanbus/include/windows")
                set_property(TARGET LinkerHand::linkerhand_cpp_sdk APPEND PROPERTY
                    INTERFACE_INCLUDE_DIRECTORIES "${_lh_thirdparty_root}/libcanbus/include/windows")
            endif()
            if(EXISTS "${_lh_thirdparty_root}/PCAN_Basic/Include")
                set_property(TARGET LinkerHand::linkerhand_cpp_sdk APPEND PROPERTY
                    INTERFACE_INCLUDE_DIRECTORIES "${_lh_thirdparty_root}/PCAN_Basic/Include")
            endif()
        elseif(UNIX AND EXISTS "${_lh_thirdparty_root}/libcanbus/include/linux")
            set_property(TARGET LinkerHand::linkerhand_cpp_sdk APPEND PROPERTY
                INTERFACE_INCLUDE_DIRECTORIES "${_lh_thirdparty_root}/libcanbus/include/linux")
        endif()
    endif()
endif()

set(linkerhand-cpp-sdk_INCLUDE_DIRS "${_lh_inc}")
set(linkerhand-cpp-sdk_LIBRARIES    LinkerHand::linkerhand_cpp_sdk)
set(linkerhand-cpp-sdk_LIBRARY_DIR  "${_lh_lib_dir}")
set(linkerhand-cpp-sdk_FOUND TRUE)

unset(_lh_self_dir)
unset(_lh_extract_root)
unset(_lh_install_prefix)
unset(_lh_inc)
unset(_lh_lib_dir)
unset(_lh_lib_search)
unset(_lh_thirdparty_root)
unset(_lh_imported_loc)
unset(_lh_imported_implib)
