@echo off
setlocal enabledelayedexpansion

set BUILD_ONLY=0
set INSTALL=0
set CLEAN=0
set SKIP_TESTS=0
set USE_LIB=0
set INSTALL_PREFIX=C:\Program Files\LinkerHandSDK
set COMPILER=mingw

if "%~1"=="-h" goto help
if "%~1"=="--help" goto help
if "%~1"=="-b" set BUILD_ONLY=1
if "%~1"=="-i" set INSTALL=1
if "%~1"=="-c" set CLEAN=1
if "%~1"=="--skip-tests" set SKIP_TESTS=1
if "%~1"=="--prefix" set INSTALL_PREFIX=%~2
if "%~1"=="--use-lib" set USE_LIB=1
if "%~1"=="--compiler" (
    set COMPILER=%~2
    if /i not "!COMPILER!"=="mingw" if /i not "!COMPILER!"=="msvc" (
        echo [ERROR] Invalid compiler: !COMPILER!
        echo Valid options: mingw, msvc
        exit /b 1
    )
)

if "%CLEAN%"=="1" (
    echo [INFO] Cleaning build directory...
    if exist build rmdir /s /q build
    if exist build_msvc rmdir /s /q build_msvc
    if exist build_msvc_bin rmdir /s /q build_msvc_bin
    echo [INFO] Done
    exit /b 0
)

echo [INFO] Checking build tools...
where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake not found
    exit /b 1
)

if /i "%COMPILER%"=="mingw" (
    echo [INFO] Using MinGW compiler
    where mingw32-make >nul 2>&1
    if errorlevel 1 (
        echo [ERROR] MinGW not found
        exit /b 1
    )
    set BUILD_DIR=build
    set CMAKE_GENERATOR="MinGW Makefiles"
    set CMAKE_MAKE_PROGRAM=mingw32-make
    if "%USE_LIB%"=="1" (
        echo [WARNING] --use-lib option is not supported for MinGW, using source build mode
        set USE_LIB=0
    )
) else if /i "%COMPILER%"=="msvc" (
    echo [INFO] Using MSVC compiler
    set BUILD_DIR=build_msvc
    if "%USE_LIB%"=="1" set BUILD_DIR=build_msvc_bin
    set CMAKE_GENERATOR="NMake Makefiles"
    set CMAKE_MAKE_PROGRAM=nmake
    
    set "VCVARS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    if not exist "!VCVARS_PATH!" (
        set "VCVARS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
    )
    if not exist "!VCVARS_PATH!" (
        set "VCVARS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    )
    if not exist "!VCVARS_PATH!" (
        set "VCVARS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
    )
    if not exist "!VCVARS_PATH!" (
        set "VCVARS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    )
    if not exist "!VCVARS_PATH!" (
        set "VCVARS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    )
    
    if not exist "!VCVARS_PATH!" (
        echo [ERROR] MSVC not found. Please install Visual Studio.
        exit /b 1
    )
    
    echo [INFO] Setting up MSVC environment...
    call "!VCVARS_PATH!"
    if errorlevel 1 (
        echo [ERROR] Failed to setup MSVC environment
        exit /b 1
    )
)

if "%USE_LIB%"=="1" (
    echo [INFO] Using pre-built library mode
    if /i "%COMPILER%"=="msvc" (
        set LIB_PATH=lib\win64\msvc\linkerhand_cpp_sdk.dll
    ) else (
        set LIB_PATH=lib\win64\mingw\linkerhand_cpp_sdk.dll
    )
    if not exist "!LIB_PATH!" (
        echo [ERROR] Pre-built library not found: !LIB_PATH!
        echo [ERROR] Please build the library first using: build.bat --compiler !COMPILER!
        exit /b 1
    )
)

echo [INFO] Creating build directory...
if exist "!BUILD_DIR!" rmdir /s /q "!BUILD_DIR!"
mkdir "!BUILD_DIR!"
cd "!BUILD_DIR!"

echo [INFO] Configuring CMake...
set CMAKE_ARGS=-G !CMAKE_GENERATOR! -DCMAKE_BUILD_TYPE=Release
if defined CMAKE_MAKE_PROGRAM set CMAKE_ARGS=!CMAKE_ARGS! -DCMAKE_MAKE_PROGRAM=!CMAKE_MAKE_PROGRAM!
if "%SKIP_TESTS%"=="1" set CMAKE_ARGS=!CMAKE_ARGS! -DBUILD_TESTS=OFF
if "%USE_LIB%"=="1" (
    set CMAKE_ARGS=!CMAKE_ARGS! -DBUILD_SO=OFF
) else (
    set CMAKE_ARGS=!CMAKE_ARGS! -DBUILD_SO=ON
)

cmake !CMAKE_ARGS! ..
if errorlevel 1 (
    echo [ERROR] CMake failed
    cd ..
    exit /b 1
)

echo [INFO] Building...
if /i "%COMPILER%"=="mingw" (
    mingw32-make
) else if /i "%COMPILER%"=="msvc" (
    nmake
)
if errorlevel 1 (
    echo [ERROR] Build failed
    cd ..
    exit /b 1
)

echo [INFO] Build successful!
echo [INFO] Executables: %CD%\bin\

if "%BUILD_ONLY%"=="1" (
    echo Usage: Run programs from bin\ directory
    cd ..
    exit /b 0
)

if "%INSTALL%"=="1" (
    echo [INFO] Installing to %INSTALL_PREFIX%...
    net session >nul 2>&1
    if errorlevel 1 (
        echo [ERROR] Need admin rights
        cd ..
        exit /b 1
    )
    if not exist "%INSTALL_PREFIX%" mkdir "%INSTALL_PREFIX%"
    if exist "%INSTALL_PREFIX%\include" rmdir /s /q "%INSTALL_PREFIX%\include"
    if exist "%INSTALL_PREFIX%\lib" rmdir /s /q "%INSTALL_PREFIX%\lib"
    xcopy /E /I /Y ..\include "%INSTALL_PREFIX%\include"
    xcopy /E /I /Y ..\lib "%INSTALL_PREFIX%\lib"
    setx LINKERHAND_SDK_ROOT "%INSTALL_PREFIX%" /M >nul 2>&1
    setx PATH "%PATH%;%INSTALL_PREFIX%\lib\win64" /M >nul 2>&1
    echo [INFO] Installation complete!
    echo Restart terminal to use environment variables
    cd ..
    exit /b 0
)

echo Usage: Run programs from bin\ directory
echo Use 'build.bat -i' to install to system
cd ..
exit /b 0

:help
echo Usage: build.bat [options]
echo Options:
echo   -h, --help         Show help
echo   -b                 Build only
echo   -i                 Build and install
echo   -c                 Clean build
echo   --skip-tests       Skip tests
echo   --prefix PATH      Install directory
echo   --compiler TYPE    Compiler type: mingw (default) or msvc
echo   --use-lib          Use pre-built library (MSVC only)
exit /b 0
