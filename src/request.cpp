#include "request.h"

Request::Request(
    const std::string& method, 
    const std::string& path,
    const std::string& body,
    const std::string& sender,
    const std::string& recipient
) 

    : method(method),
      path(path),
      body(body),
      sender(sender),
      recipient(recipient)
{
}

const std::string& Request::getMethod() const {

    return method;

}

const std::string& Request::getPath() const {

    return path;
    
}

const std::string& Request::getBody() const {

    return body;

}

const std::string& Request::getSender() const {

    return sender;

}

const std::string& Request::getRecipient() const {

    return recipient;
    
}