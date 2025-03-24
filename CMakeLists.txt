cmake_minimum_required(VERSION 3.15)

set(CMAKE_TOOLCHAIN_FILE "%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")
project("LLHTTP")

find_package(unofficial-brotli CONFIG REQUIRED)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE libs/HBuffer/include)
target_include_directories(${PROJECT_NAME} PRIVATE include/)
target_compile_definitions(${PROJECT_NAME} PUBLIC LLHTTP_USE_PCH LLHTTP_PCH_DIR=\"LLHttp/pch.h\")

target_precompile_headers(${PROJECT_NAME} PUBLIC "include/LLHttp/pch.h")

if (MSVC)
    target_sources(${PROJECT_NAME} PRIVATE src/pch.cpp)
endif()
file(GLOB SOURCES "src/*.cpp")