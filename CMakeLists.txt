cmake_minimum_required(VERSION 3.16)
project(linkerhand-legacy-cpp-sdk VERSION 2.2.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(CTest)

option(BUILD_EXAMPLES "Build the read-only SDK integration example" ON)
option(LINKERHAND_USE_TRANSPORT "Build optional transport adapter integration/tests" OFF)
if(LINKERHAND_USE_TRANSPORT)
    include(cmake/LegacyTransport.cmake)
endif()
if(BUILD_EXAMPLES)
    add_subdirectory(demo)
endif()
message(STATUS "Packaged SDK version: ${PROJECT_VERSION}; implementation is prebuilt")

# ---------------------------------------------------------------------------
# 安装规则（发布包模式）
# SDK 已是预编译产物，源码不在此处。
# cmake --install 时把 include/ 头与对应平台的 lib 铺到 prefix，与 build.sh
# 期望的目录布局保持一致：
#   $PREFIX/include/linkerhand-legacy-cpp-sdk/{api,core,communication}/*.h
#   $PREFIX/lib/linkerhand-legacy-cpp-sdk/lib/<预编译库>
# 消费者需配置运行时库搜索路径。
# ---------------------------------------------------------------------------

set(LH_INSTALL_INCLUDE_DIR "include/linkerhand-legacy-cpp-sdk")
set(LH_INSTALL_LIB_ROOT    "lib/linkerhand-legacy-cpp-sdk")

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

# CMake config 包：让下游用 find_package(linkerhand-legacy-cpp-sdk CONFIG REQUIRED)
# 配置文件相对路径自定位（见 cmake/linkerhand-legacy-cpp-sdk-config.cmake），
# prefix 改了无需重新生成。
install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/linkerhand-legacy-cpp-sdk-config.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/linkerhand-legacy-cpp-sdk-config-version.cmake"
    DESTINATION "lib/cmake/linkerhand-legacy-cpp-sdk"
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
    add_test(NAME legacy_package_versions
        COMMAND ${Python3_EXECUTABLE} -B "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_package_versions.py")
endif()

if(BUILD_TESTING)
    add_executable(sdk_contract_test tests/sdk_contract_test.cpp)
    target_include_directories(sdk_contract_test PRIVATE include/core)
    add_test(NAME sdk_contract COMMAND sdk_contract_test)
endif()
install(FILES README.md LICENSE CHANGELOG.md DESTINATION share/doc/linkerhand-legacy-cpp-sdk)
install(DIRECTORY docs/ DESTINATION share/doc/linkerhand-legacy-cpp-sdk/docs FILES_MATCHING PATTERN "*.md")
configure_file(cmake/uninstall.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake" @ONLY)
add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake")
