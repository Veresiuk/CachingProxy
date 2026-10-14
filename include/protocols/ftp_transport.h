#pragma once

#include <curl/curl.h>

#include <string>

void configureFtp(
    CURL* curl,
    const std::string& method,
    const std::string& path
);