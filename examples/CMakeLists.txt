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

# CAN-FD 总开关：
# - 嵌套模式：父 CMake 已设 USE_CANFD,直接沿用
# - 独立模式：优先跟随 find_package 拿到的 LINKERHAND_USE_CANFD（release-package
#   据 third_party/libcanbus 是否存在自动推断；安装版 Config 据 SDK 构建时的值固化）;
#   都没有再回落到 ON。用户可显式 -DUSE_CANFD=ON/OFF 覆盖。
if(NOT DEFINED USE_CANFD)
    if(DEFINED LINKERHAND_USE_CANFD)
        set(USE_CANFD ${LINKERHAND_USE_CANFD} CACHE BOOL "Enable CAN-FD examples and libcanbus runtime deps")
    else()
        option(USE_CANFD "Enable CAN-FD examples and libcanbus runtime deps" ON)
    endif()
endif()

# 自治降级：USE_CANFD=ON 但本地缺料时自动降级 OFF + 警告,避免突然 Hcanbus.h not found
# 或链接 undefined reference to CanFD::*。两个判据：
#   (1) libcanbus 头实际不在场（旧 release 包没带、或被裁过）
#   (2) 嵌套以外模式下 SDK .so 不含 CanFD 符号（往往是发布以 USE_CANFD=OFF 构建,
#       通过 third_party/libcanbus 是否随包发布间接判断）
if(USE_CANFD)
    if(WIN32)
        set(_LH_CANFD_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/libcanbus/include/windows/Hcanbus.h")
    else()
        set(_LH_CANFD_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/libcanbus/include/linux/Hcanbus.h")
    endif()
    set(_LH_CANFD_TP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/libcanbus")
    # 兜底再看安装位置 third_party
    if((NOT EXISTS "${_LH_CANFD_HEADER}" OR NOT EXISTS "${_LH_CANFD_TP_ROOT}")
       AND NOT LINKERHAND_AS_SUBPROJECT AND LINKERHAND_INCLUDE_DIR)
        get_filename_component(_LH_PREFIX "${LINKERHAND_INCLUDE_DIR}" DIRECTORY)
        get_filename_component(_LH_PREFIX "${_LH_PREFIX}" DIRECTORY)
        set(_LH_INSTALLED_TP "${_LH_PREFIX}/lib/linkerhand-cpp-sdk/third_party/libcanbus")
        if(EXISTS "${_LH_INSTALLED_TP}")
            set(_LH_CANFD_TP_ROOT "${_LH_INSTALLED_TP}")
        endif()
        if(WIN32)
            set(_LH_INSTALLED_HDR "${_LH_INSTALLED_TP}/include/windows/Hcanbus.h")
        else()
            set(_LH_INSTALLED_HDR "${_LH_INSTALLED_TP}/include/linux/Hcanbus.h")
        endif()
        if(EXISTS "${_LH_INSTALLED_HDR}")
            set(_LH_CANFD_HEADER "${_LH_INSTALLED_HDR}")
        endif()
    endif()
    set(_LH_NEED_DOWNGRADE FALSE)
    if(NOT EXISTS "${_LH_CANFD_HEADER}")
        set(_LH_NEED_DOWNGRADE TRUE)
        message(STATUS "examples: USE_CANFD downgrade reason: libcanbus header not found")
    elseif(NOT LINKERHAND_AS_SUBPROJECT AND NOT EXISTS "${_LH_CANFD_TP_ROOT}")
        # 头能找到但 third_party/libcanbus 整体不在场:发布包多半是 USE_CANFD=OFF 构建的,
        # 此时 SDK .so 不含 CanFD 符号,继续编 canfd 示例必链接失败。
        set(_LH_NEED_DOWNGRADE TRUE)
        message(STATUS "examples: USE_CANFD downgrade reason: third_party/libcanbus not shipped with this SDK")
    endif()
    # 终极判据：直接查 SDK .so 是否真带 CanFD 符号。下游有时会自己把 third_party/libcanbus
    # 补回来,但发布的 .so 仍是 USE_CANFD=OFF 编出来的——这里能兜住这种"头补了 lib 没补"的坑。
    # 仅 Linux/macOS 走 nm;Windows 暂不做,继续依赖前两条。
    if(NOT _LH_NEED_DOWNGRADE AND NOT LINKERHAND_AS_SUBPROJECT
       AND UNIX AND NOT APPLE AND LINKERHAND_LIB_DIR)
        set(_LH_SDK_SO "")
        foreach(_n linkerhand_cpp_sdk.so.2.0.0 linkerhand_cpp_sdk.so.2 linkerhand_cpp_sdk.so)
            if(NOT _LH_SDK_SO AND EXISTS "${LINKERHAND_LIB_DIR}/${_n}")
                set(_LH_SDK_SO "${LINKERHAND_LIB_DIR}/${_n}")
            endif()
        endforeach()
        if(_LH_SDK_SO)
            execute_process(
                COMMAND sh -c "nm -D --defined-only '${_LH_SDK_SO}' 2>/dev/null | grep -q CanFD"
                RESULT_VARIABLE _LH_CANFD_SYM_RC
            )
            if(NOT _LH_CANFD_SYM_RC EQUAL 0)
                set(_LH_NEED_DOWNGRADE TRUE)
                message(STATUS "examples: USE_CANFD downgrade reason: SDK lib does not export CanFD symbols (${_LH_SDK_SO})")
            endif()
        endif()
        unset(_LH_SDK_SO)
        unset(_LH_CANFD_SYM_RC)
    endif()
    if(_LH_NEED_DOWNGRADE)
        message(WARNING "USE_CANFD=ON but CAN-FD prerequisites missing; downgrading USE_CANFD=OFF for examples.")
        set(USE_CANFD OFF CACHE BOOL "Enable CAN-FD examples and libcanbus runtime deps" FORCE)
    endif()
    unset(_LH_CANFD_HEADER)
    unset(_LH_CANFD_TP_ROOT)
    unset(_LH_INSTALLED_TP)
    unset(_LH_INSTALLED_HDR)
    unset(_LH_NEED_DOWNGRADE)
endif()

if(WIN32)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/PCAN_Basic/Include)
    if(USE_CANFD)
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/libcanbus/include/windows)
    endif()
else()
    if(USE_CANFD)
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/libcanbus/include/linux)

        # Linux: 设置 libcanbus 库路径（按发行版选 ubuntu20/22，与顶层 CMakeLists 一致）
        set(LIBCANBUS_BASE "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/libcanbus/linux")
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
            set(LIBCANBUS_DIR "${LIBCANBUS_BASE}/arm64")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
            set(LIBCANBUS_DIR "${LIBCANBUS_BASE}/arm")
        else()
            # 嵌套构建时继承顶层 UBUNTU_VERSION_MAJOR；独立构建则就地读 /etc/os-release
            if(NOT DEFINED UBUNTU_VERSION_MAJOR OR UBUNTU_VERSION_MAJOR STREQUAL "")
                if(EXISTS "/etc/os-release")
                    file(STRINGS "/etc/os-release" _LH_VID_LINE REGEX "^VERSION_ID=")
                    if(_LH_VID_LINE MATCHES "([0-9]+)")
                        set(UBUNTU_VERSION_MAJOR "${CMAKE_MATCH_1}")
                    endif()
                endif()
            endif()
            if(UBUNTU_VERSION_MAJOR AND UBUNTU_VERSION_MAJOR GREATER_EQUAL 22 AND
               EXISTS "${LIBCANBUS_BASE}/ubuntu22/libcanbus.so")
                set(LIBCANBUS_DIR "${LIBCANBUS_BASE}/ubuntu22")
            elseif(EXISTS "${LIBCANBUS_BASE}/ubuntu20/libcanbus.so")
                set(LIBCANBUS_DIR "${LIBCANBUS_BASE}/ubuntu20")
            else()
                set(LIBCANBUS_DIR "${LIBCANBUS_BASE}/ubuntu22")
            endif()
        endif()

        # 添加库搜索路径
        link_directories(${LIBCANBUS_DIR})
        # libcanbus.so 是 SDK .so 的间接(NEEDED)依赖；ld 解析间接依赖只认 rpath-link，
        # 不认 -L/link_directories，故显式补 -rpath-link，避免 "libcanbus.so ... not found"
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath-link,${LIBCANBUS_DIR}")
        message(STATUS "libcanbus directory: ${LIBCANBUS_DIR}")
    endif()

    # 设置运行时库路径，确保运行时能找到 SDK .so（以及 USE_CANFD 时的 libcanbus.so）
    # $ORIGIN: 让 exe 在自身所在目录(bin/)查找,从而拿到 copy_dependencies 复制过来的 SDK .so
    set(CMAKE_INSTALL_RPATH "\$ORIGIN")
    if(USE_CANFD AND LIBCANBUS_DIR)
        list(APPEND CMAKE_INSTALL_RPATH "${LIBCANBUS_DIR}")
    endif()
    if(LINKERHAND_LIB_DIR)
        list(APPEND CMAKE_INSTALL_RPATH "${LINKERHAND_LIB_DIR}")
    endif()
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
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

# 所有 example 共享同一份运行时依赖：用单一 custom target 一次性拷贝,
# 而不是给每个 exe 挂 POST_BUILD —— 否则 ~30 个 example 会重复刷屏并和
# SONAME 软链 target 在并行构建下争抢同一份文件。
function(_define_examples_runtime_deps_target)
    if(TARGET examples_runtime_deps)
        return()
    endif()

    set(_cmds "")

    if(LINKERHAND_AS_SUBPROJECT)
        list(APPEND _cmds
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:linkerhand_cpp_sdk>"
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    elseif(LINKERHAND_DLL AND EXISTS "${LINKERHAND_DLL}")
        list(APPEND _cmds
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${LINKERHAND_DLL}"
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    endif()

    if(WIN32)
        set(PCAN_DLL "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/PCAN_Basic/x64/PCANBasic.dll")
        if(EXISTS "${PCAN_DLL}")
            list(APPEND _cmds
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${PCAN_DLL}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
        endif()

        set(CANFD_DLL "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/libcanbus/windows/x64/HCanbus.dll")
        if(USE_CANFD AND EXISTS "${CANFD_DLL}")
            list(APPEND _cmds
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CANFD_DLL}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
        endif()

        # 拷贝 mingw runtime DLL：SDK DLL 与 exe 必须共享同一份 libstdc++，
        # 否则跨 DLL 边界传 std::string / std::vector 会因 allocator 不一致引发
        # heap corruption；exe 启动若找不到 runtime 也会被 Windows 静默终止。
        # win32 线程模型不依赖 libwinpthread-1.dll，所以不在拷贝列表中。
        foreach(_RT_DLL libstdc++-6.dll libgcc_s_seh-1.dll)
            set(_RT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../lib/win64/mingw/${_RT_DLL}")
            if(EXISTS "${_RT_PATH}")
                list(APPEND _cmds
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${_RT_PATH}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
            endif()
        endforeach()
    elseif(UNIX AND NOT APPLE)
        if(USE_CANFD AND DEFINED LIBCANBUS_DIR)
            set(CANBUS_SO "${LIBCANBUS_DIR}/libcanbus.so")
            if(EXISTS "${CANBUS_SO}")
                list(APPEND _cmds
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${CANBUS_SO}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
            endif()
            # libusb-1.0.so 是 libcanbus 的依赖
            set(LIBUSB_SO "${LIBCANBUS_DIR}/libusb-1.0.so")
            if(EXISTS "${LIBUSB_SO}")
                list(APPEND _cmds
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${LIBUSB_SO}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
            endif()
        endif()
    endif()

    if(LINKERHAND_AS_SUBPROJECT)
        add_custom_target(examples_runtime_deps ALL
            ${_cmds}
            DEPENDS linkerhand_cpp_sdk
            COMMENT "Staging examples runtime dependencies into ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
            VERBATIM
        )
    else()
        add_custom_target(examples_runtime_deps ALL
            ${_cmds}
            COMMENT "Staging examples runtime dependencies into ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
            VERBATIM
        )
    endif()
endfunction()

# Unix: 复制只拿到 .so.<VERSION>,exe NEEDED 的是 .so.<SOVERSION>(由 SONAME 决定),
# 必须在 bin/ 里同步建出 SONAME 软链,否则 ld.so 找不到。
# 用单一 custom target 而非 per-example POST_BUILD：并行构建下多 target
# 同时跑 remove + create_symlink 会出现 "File exists" 竞态。
function(_define_examples_sdk_soname_symlink_target)
    if(TARGET examples_sdk_soname_symlink)
        return()
    endif()
    if(NOT LINKERHAND_AS_SUBPROJECT)
        return()
    endif()
    if(NOT (UNIX AND NOT APPLE))
        return()
    endif()
    add_custom_target(examples_sdk_soname_symlink ALL
        COMMAND ${CMAKE_COMMAND} -E remove -f
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_SONAME_FILE_NAME:linkerhand_cpp_sdk>"
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            "$<TARGET_FILE_NAME:linkerhand_cpp_sdk>"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_SONAME_FILE_NAME:linkerhand_cpp_sdk>"
        DEPENDS linkerhand_cpp_sdk
        COMMENT "Creating SONAME symlink for linkerhand_cpp_sdk in output directory"
        VERBATIM
    )
endfunction()

function(copy_dependencies target)
    _define_examples_runtime_deps_target()
    _define_examples_sdk_soname_symlink_target()
    add_dependencies(${target} examples_runtime_deps)
    if(TARGET examples_sdk_soname_symlink)
        add_dependencies(${target} examples_sdk_soname_symlink)
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
    if(USE_CANFD AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
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
