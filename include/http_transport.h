#pragma once

#include <string>

struct HttpResponse {

    int statusCode = 0;
    std::string headers;
    std::string body;

};

class HttpTransport {

    public:

        virtual ~HttpTransport() = default;

        virtual HttpResponse send(
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
        ) = 0;
};