#pragma once

#include <string>

class Request {

    private:

    std::string method;
    std::string path;

    public:

    Request(const std::string& method, const std::string& path);

    std::string getMethod() const;
    std::string getPath() const;

};