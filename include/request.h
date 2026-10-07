#pragma once

#include <string>

class Request {

    private:
    std::string method;
    std::string path;
    std::string body;
    std::string sender;
    std::string recipient;

    public:

    Request(
        const std::string& method, 
        const std::string& path,
        const std::string& body = "",
        const std::string& sender = "",
        const std::string& recipient = ""
    );

    const std::string& getMethod() const;
    const std::string& getPath() const;
    const std::string& getBody() const;
    const std::string& getSender() const;
    const std::string& getRecipient() const; 

};