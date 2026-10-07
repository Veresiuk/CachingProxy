#pragma once

#include <string>

#include "http_transport.h"

class CurlTransport : public HttpTransport {

public:

    HttpResponse send(
        const std::string& scheme,
        const std::string& host,
        int port,
        const std::string& username,
        const std::string& password,
        const std::string& method,
        const std::string& path,
        const std::string& body,
        const std::string& sender,
        const std::string& recipient
    ) override;

};