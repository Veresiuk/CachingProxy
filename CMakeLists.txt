cmake_minimum_required(VERSION 3.10)

project(CachingProxy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH "C:/dev/vcpkg/installed/x64-windows")
set(CMAKE_MAP_IMPORTED_CONFIG_DEBUG Release)


find_package(CURL CONFIG REQUIRED)

add_executable(CachingProxy
    src/main.cpp
    src/Cache.cpp
    src/http_server.cpp
    src/http_client.cpp
    src/curl_transport.cpp 
    src/request.cpp
)

add_executable(CurlDiagnostic
    src/curl_diagnostic.cpp
)

target_include_directories(CurlDiagnostic PRIVATE
    "C:/dev/vcpkg/installed/x64-windows/include"
)

target_link_directories(CurlDiagnostic PRIVATE
    "C:/dev/vcpkg/installed/x64-windows/lib"
)

target_link_libraries(CurlDiagnostic PRIVATE
    CURL::libcurl
)

add_custom_command(
    TARGET CurlDiagnostic
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/dev/vcpkg/installed/x64-windows/bin/libcurl.dll"
        "$<TARGET_FILE_DIR:CurlDiagnostic>/libcurl.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/dev/vcpkg/installed/x64-windows/bin/z.dll"
        "$<TARGET_FILE_DIR:CurlDiagnostic>/z.dll"
)

target_include_directories(CachingProxy PRIVATE include)

target_link_libraries(CachingProxy PRIVATE winhttp CURL::libcurl)

add_custom_command(
    TARGET CachingProxy
    POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/dev/vcpkg/installed/x64-windows/bin/libcurl.dll"
        "$<TARGET_FILE_DIR:CachingProxy>/libcurl.dll"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:/dev/vcpkg/installed/x64-windows/bin/z.dll"
        "$<TARGET_FILE_DIR:CachingProxy>/z.dll"
)