cmake_minimum_required(VERSION 3.15)
project(ReverseProxy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 使用現代 Boost CMake Config 模式
find_package(Boost CONFIG REQUIRED)

# 包含我們的 header
include_directories(${PROJECT_SOURCE_DIR}/include)

# Debug flags for Linux
if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")  # Debug symbols, no optimization
endif()

# 三個程式的設定
add_executable(expose src/expose.cpp)
target_link_libraries(expose PRIVATE Boost::boost)

add_executable(proxy_server src/proxy_server.cpp)
target_link_libraries(proxy_server PRIVATE Boost::boost)

add_executable(echo_server src/echo_server.cpp)
target_link_libraries(echo_server PRIVATE Boost::boost)
