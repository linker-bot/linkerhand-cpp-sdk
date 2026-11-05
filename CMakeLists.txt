cmake_minimum_required(VERSION 3.16)
project(LinkerHandSDKExamples LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(CTest)

option(BUILD_EXAMPLES "Build example applications" ON)
option(LINKERHAND_USE_TRANSPORT "Enable the optional transport callback adapter" OFF)
if(LINKERHAND_USE_TRANSPORT)
    include(cmake/LegacyTransport.cmake)
endif()
option(BUILD_HAND_TEACH_PENDANT "Deprecated: external teach pendant source is not packaged" OFF)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_HAND_TEACH_PENDANT)
    message(FATAL_ERROR "hand_teach_pendant source is not packaged; use webui or hand_tui")
endif()

message(STATUS "=== LinkerHand C++ SDK Examples ===")
message(STATUS "Packaged SDK version: 2.2.1")
message(STATUS "Build Examples: ${BUILD_EXAMPLES}")
message(STATUS "Build Hand Teach Pendant: ${BUILD_HAND_TEACH_PENDANT}")
message(STATUS "=====================================")

# ---------------------------------------------------------------------------
# 安装规则（发布包模式）
# 本目录是 pack.sh 产出的发布包：SDK 已是预编译产物，源码不在此处。
# cmake --install 时把 include/ 头与对应平台的 lib 铺到 prefix，与 build.sh
# 期望的目录布局保持一致：
#   $PREFIX/include/linkerhand-cpp-sdk/{api,core,communication}/*.h
#   $PREFIX/lib/linkerhand-cpp-sdk/lib/<预编译库>
# build.sh 随后在 $PREFIX/lib/ 建符号链接 + ldconfig。
# ---------------------------------------------------------------------------

set(LH_INSTALL_INCLUDE_DIR "include/linkerhand-cpp-sdk")
set(LH_INSTALL_LIB_ROOT    "lib/linkerhand-cpp-sdk")

# 头：按子目录白名单发布
install(DIRECTORY include/api/          DESTINATION "${LH_INSTALL_INCLUDE_DIR}/api"           FILES_MATCHING PATTERN "*.h")
install(DIRECTORY include/core/         DESTINATION "${LH_INSTALL_INCLUDE_DIR}/core"          FILES_MATCHING PATTERN "*.h")
install(DIRECTORY include/communication/ DESTINATION "${LH_INSTALL_INCLUDE_DIR}/communication" FILES_MATCHING PATTERN "*.h")

# 平台/架构选库：与 lib/ 实际目录布局一致
set(LH_LIB_SRC_DIR "")
if(WIN32)
    if(MSVC)
        set(LH_LIB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/win64/msvc")
    else()
        set(LH_LIB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/win64/mingw")
    endif()
elseif(UNIX AND NOT APPLE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
        set(LH_LIB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/x86_64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$")
        set(LH_LIB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/arm64")
    endif()
endif()

if(LH_LIB_SRC_DIR AND EXISTS "${LH_LIB_SRC_DIR}")
    # 用 install(DIRECTORY) 而非 install(FILES)：保留 .so → .so.2 → .so.2.2.1 的软链
    install(DIRECTORY "${LH_LIB_SRC_DIR}/" DESTINATION "${LH_INSTALL_LIB_ROOT}/lib" USE_SOURCE_PERMISSIONS)
    message(STATUS "Install lib source: ${LH_LIB_SRC_DIR}")
else()
    message(WARNING "No prebuilt lib matched (system=${CMAKE_SYSTEM_NAME} arch=${CMAKE_SYSTEM_PROCESSOR}); install will skip libs.")
endif()

# CMake config 包：让下游用 find_package(linkerhand-cpp-sdk CONFIG REQUIRED)
# 配置文件相对路径自定位（见 cmake/linkerhand-cpp-sdk-config.cmake），
# prefix 改了无需重新生成。
install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/linkerhand-cpp-sdk-config.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/linkerhand-cpp-sdk-config-version.cmake"
    DESTINATION "lib/cmake/linkerhand-cpp-sdk"
)

# third_party 头：CanFD.h / PCANBus.h / CanFrame.h 公共头泄漏了 Hcanbus.h / PCANBasic.h，
# 任何 include 它们的下游都得有这条路径，所以随包发布。
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/"
            DESTINATION "${LH_INSTALL_LIB_ROOT}/third_party")
endif()

install(DIRECTORY include/legacy/ DESTINATION "${LH_INSTALL_INCLUDE_DIR}/legacy" FILES_MATCHING PATTERN "*.h")

if(LINKERHAND_USE_TRANSPORT AND BUILD_TESTING)
    enable_testing()
    add_executable(legacy_callbacks_test tests/can_callbacks_test.cpp)
    target_link_libraries(legacy_callbacks_test PRIVATE linkerhand_legacy_transport)
    add_test(NAME legacy_callbacks COMMAND legacy_callbacks_test)
endif()

if(BUILD_TESTING)
    enable_testing()
    add_executable(legacy_modbus_test tests/modbus_callbacks_test.cpp)
    target_include_directories(legacy_modbus_test PRIVATE include)
    target_compile_features(legacy_modbus_test PRIVATE cxx_std_17)
    add_test(NAME legacy_modbus COMMAND legacy_modbus_test)
endif()

if(BUILD_TESTING)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    add_test(NAME legacy_scope
        COMMAND ${Python3_EXECUTABLE} -B -m unittest discover -s tests -p test_legacy_scope.py -v)
    set_tests_properties(legacy_scope PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    add_test(NAME legacy_package_versions
        COMMAND ${Python3_EXECUTABLE} -B "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_package_versions.py")
endif()
