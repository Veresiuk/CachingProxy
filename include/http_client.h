#pragma once

#include <string>
#include "request.h"

struct HttpResponse {

    int statusCode;
    std::string headers;
    std::string body;

};

class HttpClient {

    private:

    std::string origin;

    public:

    HttpClient(const std::string& origin);

    HttpResponse sendRequest(const Request& request);

};