# linkerhand-cpp-sdk-config-version.cmake
# CMake find_package 版本兼容性检查。语义：同主版本号兼容（SOVERSION=2）。

set(PACKAGE_VERSION "2.0.0")

if(PACKAGE_FIND_VERSION_RANGE)
    # 区间形式 find_package(... 2.0...3.0)：交给 CMake 自己判断
    if(PACKAGE_FIND_VERSION_MIN_MAJOR EQUAL 2)
        set(PACKAGE_VERSION_COMPATIBLE TRUE)
    else()
        set(PACKAGE_VERSION_COMPATIBLE FALSE)
    endif()
elseif(PACKAGE_FIND_VERSION)
    if(PACKAGE_FIND_VERSION_MAJOR EQUAL 2 AND
       NOT PACKAGE_FIND_VERSION VERSION_GREATER PACKAGE_VERSION)
        set(PACKAGE_VERSION_COMPATIBLE TRUE)
        if(PACKAGE_FIND_VERSION VERSION_EQUAL PACKAGE_VERSION)
            set(PACKAGE_VERSION_EXACT TRUE)
        endif()
    else()
        set(PACKAGE_VERSION_COMPATIBLE FALSE)
    endif()
else()
    # 未指定版本：默认兼容
    set(PACKAGE_VERSION_COMPATIBLE TRUE)
endif()
