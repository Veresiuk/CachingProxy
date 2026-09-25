cmake_minimum_required(VERSION 3.10)

project(CachingProxy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(CachingProxy
    src/main.cpp
    src/Cache.cpp
    src/http_server.cpp
    src/http_client.cpp
    src/request.cpp
)

target_include_directories(CachingProxy PRIVATE include)

target_link_libraries(CachingProxy PRIVATE winhttp)