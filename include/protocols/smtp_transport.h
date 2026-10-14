#pragma once

#include <curl/curl.h>

#include <string>

void configureSmtp(
    CURL* curl,
    const std::string& sender,
    const std::string& recipient,
    curl_slist** recipients
);

void configureSmtps(
    CURL* curl,
    const std::string& sender,
    const std::string& recipient,
    curl_slist** recipients
);