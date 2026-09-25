#include "request.h"

Request::Request(const std::string& method, const std::string& path) {

    this->method = method;
    this->path = path;

}

std::string Request::getMethod() const {

    return method;

}

std::string Request::getPath() const {

    return path;
    
}