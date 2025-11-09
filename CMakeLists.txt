cmake_minimum_required(VERSION 3.16)
project(financial_accounting_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE FINANCIAL_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(financial_app ${FINANCIAL_SOURCES})

target_include_directories(financial_app PRIVATE include)

target_compile_options(financial_app PRIVATE -Wall -Wextra -Wpedantic)
