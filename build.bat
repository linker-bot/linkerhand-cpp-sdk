@echo off
setlocal EnableExtensions
rem Build consumers of the prebuilt SDK; implementation source is not packaged.
set "SDK_SOURCE=%~dp0"
set "SDK_BUILD=%~dp0build"
set "SDK_PREFIX=%LOCALAPPDATA%\LinkerHandSDK"
set "SDK_TESTS=ON"
set "SDK_INSTALL=0"
set "SDK_RUN_TESTS=0"
set "SDK_GENERATOR="
:parse
if "%~1"=="" goto configure
if "%~1"=="-h" goto help
if "%~1"=="--help" goto help
if "%~1"=="-b" goto next
if "%~1"=="--build" goto next
if "%~1"=="--use-lib" goto next
if "%~1"=="-i" (set "SDK_INSTALL=1"& goto next)
if "%~1"=="--install" (set "SDK_INSTALL=1"& goto next)
if "%~1"=="-t" (set "SDK_RUN_TESTS=1"& goto next)
if "%~1"=="--skip-tests" (set "SDK_TESTS=OFF"& goto next)
if "%~1"=="-c" (if exist "%SDK_BUILD%" rmdir /s /q "%SDK_BUILD%"
exit /b 0)
if "%~1"=="--prefix" (if "%~2"=="" exit /b 2
set "SDK_PREFIX=%~2"
shift
 goto next)
if "%~1"=="--compiler" (if /i "%~2"=="mingw" (set SDK_GENERATOR=-G "MinGW Makefiles") else if /i not "%~2"=="msvc" (exit /b 2)
shift
 goto next)
echo Unknown argument: %~1
exit /b 2
:next
shift
goto parse
:configure
cmake -S "%SDK_SOURCE%." -B "%SDK_BUILD%" %SDK_GENERATOR% -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=%SDK_TESTS% -DCMAKE_INSTALL_PREFIX="%SDK_PREFIX%"
if errorlevel 1 exit /b 1
cmake --build "%SDK_BUILD%" --config Release --parallel 4
if errorlevel 1 exit /b 1
if "%SDK_RUN_TESTS%"=="1" (ctest --test-dir "%SDK_BUILD%" -C Release --output-on-failure
if errorlevel 1 exit /b 1)
if "%SDK_INSTALL%"=="1" (cmake --install "%SDK_BUILD%" --config Release --prefix "%SDK_PREFIX%"
if errorlevel 1 exit /b 1)
exit /b 0
:help
echo Usage: build.bat [-b] [-t] [-i] [-c] [--skip-tests] [--prefix PATH] [--compiler msvc^|mingw]
echo Builds the read-only integration example and offline SDK tests.
echo Interactive applications are maintained in linkerbot-hand-tools.
exit /b 0
