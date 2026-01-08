cmake_minimum_required(VERSION 3.5)
project(MyProject)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(LIB_SUBDIR "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(LIB_SUBDIR "aarch64")
else()
    message(WARNING "Unknown architecture, defaulting to x86_64")
    set(LIB_SUBDIR "x86_64")
endif()

#-----------------------------------------------------------------------------
# LINKER_HAND_CPP_SDK
#-----------------------------------------------------------------------------
find_library(LINKER_HAND_LIB
    NAMES linkerhand_cpp_sdk linkerhand_cpp
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib/${LIB_SUBDIR}
        /usr/local/lib/linkerhand-cpp-sdk/${LIB_SUBDIR}
        /usr/lib/linkerhand-cpp-sdk/${LIB_SUBDIR}
        ${CMAKE_INSTALL_PREFIX}/lib/linkerhand-cpp-sdk/${LIB_SUBDIR}
    NO_DEFAULT_PATH
)

set(LINKER_HAND_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    /usr/local/include/linkerhand-cpp-sdk
    /usr/include/linkerhand-cpp-sdk
    ${CMAKE_INSTALL_PREFIX}/include/linkerhand-cpp-sdk
)

if(NOT LINKER_HAND_LIB)
    message(FATAL_ERROR "linkerhand_cpp_sdk library not found!")
endif()

if(NOT LINKER_HAND_INCLUDE_DIR)
    message(FATAL_ERROR "LinkerHand headers not found!")
endif()

message(STATUS "Found linkerhand_cpp_sdk library: ${LINKER_HAND_LIB}")
message(STATUS "Found LinkerHand headers: ${LINKER_HAND_INCLUDE_DIR}")

#-----------------------------------------------------------------------------
# INCLUDE_DIRECTORIES
#-----------------------------------------------------------------------------
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LINKER_HAND_INCLUDE_DIR}
)

#-----------------------------------------------------------------------------
# EXECUTABLE
#-----------------------------------------------------------------------------
add_executable(my_project src/main.cpp)
target_link_libraries(my_project ${LINKER_HAND_LIB} pthread)

add_executable(toolset_example examples/toolset_example.cpp)
target_link_libraries(toolset_example ${LINKER_HAND_LIB} pthread)

add_executable(action_group_show_l10 examples/action_group_show_l10.cpp)
target_link_libraries(action_group_show_l10 ${LINKER_HAND_LIB} pthread)
