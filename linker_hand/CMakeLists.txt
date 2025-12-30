cmake_minimum_required(VERSION 3.10)
project(linkerhand-cpp-sdk 
    VERSION 1.0.0
    DESCRIPTION "LinkerHand C++ SDK for robotic hand control"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

option(USE_RMAN "Enable RMAN robotic arm library support" OFF)
option(USE_ETHERCAT "Enable EtherCAT communication support" OFF)
option(BUILD_EXAMPLES "Build example applications" ON)
option(BUILD_TESTS "Build test applications" ON)
option(BUILD_SO "BUILDSO" ON)

if(WIN32)
    set(SYSTEM_ARCH "Windows")
    message(STATUS "Detected Windows system")
elseif(UNIX AND NOT APPLE)
    set(SYSTEM_ARCH "Unix/Linux")
    message(STATUS "Detected Unix/Linux system")
elseif(APPLE)
    set(SYSTEM_ARCH "macOS")
    message(STATUS "Detected macOS system")
else()
    set(SYSTEM_ARCH "Unknown")
    message(WARNING "Unknown system architecture")
endif()

execute_process(
    COMMAND lsb_release -rs
    OUTPUT_VARIABLE UBUNTU_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Ubuntu version: ${UBUNTU_VERSION}")

if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm")
  set(LINKERHAND_LIB_NAME "${PROJECT_NAME}_arm_${UBUNTU_VERSION}")
else()
  set(LINKERHAND_LIB_NAME "${PROJECT_NAME}_x86_${UBUNTU_VERSION}")
endif()

set(LINKERHAND_LIB_NAME "linkerhand_cpp")

#-----------------------------------------------------------------------------
# EtherCAT
#-----------------------------------------------------------------------------
if(USE_ETHERCAT)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ECAT REQUIRED libethercat)
    message(STATUS "EtherCAT support enabled")
else()
    message(STATUS "EtherCAT support disabled")
endif()

#-----------------------------------------------------------------------------
# PCAN
#-----------------------------------------------------------------------------
if(WIN32)
    find_library(PCAN_API_LIB 
        NAMES PCANBasic 
        PATHS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/PCAN_Basic/x64/VC_LIB"
        NO_DEFAULT_PATH
    )
    
    if(NOT PCAN_API_LIB)
        message(FATAL_ERROR "PCANBasic library not found. Please check the installation.")
    else()
        message(STATUS "Found PCAN library: ${PCAN_API_LIB}")
    endif()
endif()

#-----------------------------------------------------------------------------
# RMAN
#-----------------------------------------------------------------------------
if(USE_RMAN)
    if(WIN32)
        find_library(RMAN_API_LIB 
            NAMES api_cpp 
            PATHS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/Robotic_Arm/windows/win_mingw64_c++_v1.1.0"
            NO_DEFAULT_PATH
        )
    elseif(UNIX AND NOT APPLE)
        find_library(RMAN_API_LIB 
            NAMES api_cpp 
            PATHS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/Robotic_Arm/linux/lib"
            NO_DEFAULT_PATH
        )
    endif()
    
    if(NOT RMAN_API_LIB)
        message(FATAL_ERROR "RMAN library not found. USE_RMAN=ON but api_cpp library is missing.")
    else()
        message(STATUS "Found RMAN library: ${RMAN_API_LIB}")
    endif()
else()
    message(STATUS "RMAN support disabled")
endif()

#=============================================================================
file(GLOB_RECURSE LINKER_HAND_SOURCES 
    "src/*.cpp"
    "src/*.c"
)

if(NOT USE_ETHERCAT)
    list(REMOVE_ITEM LINKER_HAND_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/EtherCAT.cpp")
endif()

#=============================================================================
if(BUILD_SO)
    # 构建共享库模式
    set(LINKERHAND_LIB "${LINKERHAND_LIB_NAME}")
    add_library(${LINKERHAND_LIB} SHARED ${LINKER_HAND_SOURCES})

    set_target_properties(${LINKERHAND_LIB} PROPERTIES
        OUTPUT_NAME ${LINKERHAND_LIB}
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib
    )

    # 包含目录设置
    target_include_directories(${LINKERHAND_LIB}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    # 条件化包含目录
    if(USE_ETHERCAT)
        target_include_directories(${LINKERHAND_LIB} PRIVATE ${ECAT_INCLUDE_DIRS})
    endif()

    if(USE_RMAN)
        target_include_directories(${LINKERHAND_LIB} PRIVATE 
            "${CMAKE_CURRENT_SOURCE_DIR}/third_party/Robotic_Arm/include"
        )
    endif()

    if(WIN32)
        target_include_directories(${LINKERHAND_LIB} PRIVATE 
            "${CMAKE_CURRENT_SOURCE_DIR}/third_party/PCAN_Basic/Include"
        )
    endif()

    # 链接库设置
    if(WIN32)
        target_link_libraries(${LINKERHAND_LIB} PRIVATE ${PCAN_API_LIB})
    else()
        target_link_libraries(${LINKERHAND_LIB} PRIVATE pthread)
    endif()

    if(USE_ETHERCAT)
        target_link_libraries(${LINKERHAND_LIB} PRIVATE ${ECAT_LIBRARIES})
    endif()

    if(USE_RMAN)
        target_link_libraries(${LINKERHAND_LIB} PRIVATE ${RMAN_API_LIB})
    endif()

    # 编译定义
    target_compile_definitions(${LINKERHAND_LIB} PRIVATE
        USE_RMAN=$<IF:$<BOOL:${USE_RMAN}>,1,0>
        USE_ETHERCAT=$<IF:$<BOOL:${USE_ETHERCAT}>,1,0>
        SYSTEM_ARCH="${SYSTEM_ARCH}"
    )

    install(TARGETS ${LINKERHAND_LIB}
        EXPORT LinkerHandTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${PROJECT_NAME}/lib
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${PROJECT_NAME}/lib
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    )

else()
    # 使用预编译库模式
    set(LINKERHAND_LIB_FILE "${CMAKE_CURRENT_SOURCE_DIR}/lib/lib${LINKERHAND_LIB_NAME}.so")
    
    # 检查预编译库是否存在
    if(NOT EXISTS "${LINKERHAND_LIB_FILE}")
        message(FATAL_ERROR "Pre-built library not found: ${LINKERHAND_LIB_FILE}")
    endif()
    
    # 创建导入目标
    add_library(${LINKERHAND_LIB_NAME} SHARED IMPORTED)
    set_target_properties(${LINKERHAND_LIB_NAME} PROPERTIES
        IMPORTED_LOCATION "${LINKERHAND_LIB_FILE}"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
    )
    
    # 设置接口链接库
    if(UNIX AND NOT APPLE)
        set_target_properties(${LINKERHAND_LIB_NAME} PROPERTIES
            INTERFACE_LINK_LIBRARIES "pthread"
        )
    endif()
    
    set(LINKERHAND_LIB ${LINKERHAND_LIB_NAME})

    install(FILES ${LINKERHAND_LIB_FILE}
            DESTINATION lib/${PROJECT_NAME}
            RENAME lib${LINKERHAND_LIB_NAME}.so)
endif()

#=============================================================================
if(BUILD_EXAMPLES)
    add_executable(Demo_ModbusRTU examples/Demo_ModbusRTU.cpp)
    target_link_libraries(Demo_ModbusRTU PRIVATE ${LINKERHAND_LIB})

    add_executable(linker_hand_example examples/Examples.cpp)
    target_link_libraries(linker_hand_example PRIVATE ${LINKERHAND_LIB})

    add_executable(action_group_show examples/L10/action_group_show.cpp)
    target_link_libraries(action_group_show PRIVATE ${LINKERHAND_LIB})

    if(USE_RMAN)
        set_target_properties(Demo_ModbusRTU PROPERTIES
            INSTALL_RPATH "../third_party/Robotic_Arm/lib"
        )
    endif()
endif()

#=============================================================================
if(BUILD_TESTS)
    add_executable(test examples/test.cpp)
    target_link_libraries(test PRIVATE ${LINKERHAND_LIB})
    
    add_executable(test_l7 examples/test_l7.cpp)
    target_link_libraries(test_l7 PRIVATE ${LINKERHAND_LIB})
endif()

#=============================================================================
include(GNUInstallDirs)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/include
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY third_party/
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}/third_party
)

install(FILES config/setting.yaml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/config
)

if(BUILD_EXAMPLES)
    install(TARGETS linker_hand_example Demo_ModbusRTU action_group_show
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

if(BUILD_TESTS)
    install(TARGETS test test_l7
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

install(EXPORT LinkerHandTargets
    FILE LinkerHandTargets.cmake
    NAMESPACE LinkerHand::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LinkerHand
)

#=============================================================================
message(STATUS "")
message(STATUS "=== LinkerHand C++ SDK Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "System: ${SYSTEM_ARCH}")
message(STATUS "BUILD SO: ${BUILD_SO}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Examples: ${BUILD_EXAMPLES}")
message(STATUS "Build Tests: ${BUILD_TESTS}")
message(STATUS "RMAN Support: ${USE_RMAN}")
message(STATUS "EtherCAT Support: ${USE_ETHERCAT}")
message(STATUS "Build Shared Library: ${BUILD_SO}")
message(STATUS "=========================================")
message(STATUS "")
