cmake_minimum_required(VERSION 3.12)

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")
project("LLHttp")
# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)

# Ensure the compiler supports the chosen standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "src/*.cpp")
add_library(${PROJECT_NAME} STATIC ${SOURCES})

find_package(unofficial-brotli CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::brotli::brotlidec)
target_include_directories(${PROJECT_NAME} PRIVATE unofficial::brotli::brotlidec ${CMAKE_CURRENT_SOURCE_DIR}/include/LLHttp/ PUBLIC ${CMAKE_SOURCE_DIR}/libs/HBuffer/include/ ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_compile_definitions(${PROJECT_NAME} PRIVATE LLHTTP_USE_PCH LLHTTP_PCH_DIR=\"LLHttp/pch.h\")

target_precompile_headers(${PROJECT_NAME} PRIVATE "include/LLHttp/pch.h")

if (MSVC)
    target_sources(${PROJECT_NAME} PRIVATE src/pch.cpp)
endif() 