#!/usr/bin/env bash
# Build/package consumers of the prebuilt SDK. Does not rebuild the SDK binary.
set -eu
script_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
build_dir="$script_dir/build"
prefix=/usr/local
install_sdk=false
run_tests=false
skip_tests=false
uninstall_sdk=false
while [[ $# -gt 0 ]]; do
    case "$1" in
        -h|--help)
            echo "Usage: $0 [-b|--build] [-t|--test] [-i|--install] [-u|--uninstall] [-c|--clean] [--skip-tests] [--prefix PATH]"
            echo "Builds the read-only integration example and offline SDK tests. Interactive tools are in linkerbot-hand-tools."
            exit 0 ;;
        -b|--build) shift ;;
        -t|--test) run_tests=true; shift ;;
        -i|--install) install_sdk=true; shift ;;
        -u|--uninstall) uninstall_sdk=true; shift ;;
        --skip-tests) skip_tests=true; shift ;;
        --prefix) [[ $# -ge 2 ]] || { echo "Missing prefix" >&2; exit 2; }; prefix="$2"; shift 2 ;;
        -c|--clean) rm -rf -- "$build_dir"; exit 0 ;;
        *) echo "Unknown argument: $1" >&2; exit 2 ;;
    esac
done
if [[ "$(uname -s)" != Linux ]]; then
    echo "The packaged SDK supports Linux and Windows, not macOS. Use CMake directly for portable offline tests." >&2
    exit 1
fi
if [[ "$uninstall_sdk" == true ]]; then
    cmake --build "$build_dir" --target uninstall
    exit 0
fi
tests=ON
[[ "$skip_tests" == false ]] || tests=OFF
cmake -S "$script_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING="$tests" -DCMAKE_INSTALL_PREFIX="$prefix"
cmake --build "$build_dir" --parallel "${CMAKE_BUILD_PARALLEL_LEVEL:-4}"
if [[ "$run_tests" == true ]]; then ctest --test-dir "$build_dir" --output-on-failure; fi
if [[ "$install_sdk" == true ]]; then cmake --install "$build_dir" --prefix "$prefix"; fi
