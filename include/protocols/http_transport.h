#pragma once

#include <curl/curl.h>

#include <cstddef>
#include <string>

void configureHttp(
    CURL* curl,
    const std::string& method,
    const std::string& body
);

void configureHttps(
    CURL* curl,
    const std::string& method,
    const std::string& body
);

std::string normalizeHttpHeaders(
    const std::string& rawHeaders,
    std::size_t bodySize
);