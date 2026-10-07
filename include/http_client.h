#pragma once

#include <string>
#include "request.h"
#include "curl_transport.h"

class HttpClient {

    private:

    std::string origin;
    CurlTransport transport;

    public:

    explicit HttpClient(const std::string& origin);

    HttpResponse sendRequest(const Request& request);

    bool isCacheable(const Request& request) const;

};