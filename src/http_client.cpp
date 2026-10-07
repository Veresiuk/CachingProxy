#include "http_client.h"

#include <unordered_map>
#include <algorithm>
#include <cctype>

namespace {

struct ProtocolConfig {
    int port;
};

bool getProtocolConfig(
    const std::string& scheme,
    ProtocolConfig& config
) {
    static const std::unordered_map<
        std::string,
        ProtocolConfig
    > protocols = {
        {"http",  {80}},
        {"https", {443}},
        {"ftp",   {21}},
        {"smtp",  {25}},
        {"smtps", {465}}
    };

    auto it = protocols.find(scheme);

    if (it == protocols.end()) {
        return false;
    }

    config = it->second;

    return true;
}

std::string toLowerCase(
    const std::string& value
) {
    std::string result = value;

    std::transform(
        result.begin(),
        result.end(),
        result.begin(),
        [](unsigned char c) {
            return static_cast<char>(
                std::tolower(c)
            );
        }
    );

    return result;
}

}

HttpClient::HttpClient(
    const std::string& origin
)
    : origin(origin)
{
}

bool HttpClient::isCacheable(
    const Request& request
) const {

    if (request.getMethod() != "GET") {
        return false;
    }

    size_t schemePosition =
        origin.find("://");

    if (schemePosition ==
        std::string::npos) {

        return false;
    }

    std::string scheme =
        origin.substr(
            0,
            schemePosition
        );

    scheme = toLowerCase(scheme);

    return scheme != "smtp" &&
           scheme != "smtps";
}

HttpResponse HttpClient::sendRequest(
    const Request& request
) {
    size_t schemePosition =
        origin.find("://");

    if (schemePosition ==
        std::string::npos) {

        return {};
    }

    std::string scheme =
        origin.substr(
            0,
            schemePosition
        );

    scheme =
        toLowerCase(scheme);

    ProtocolConfig config;

    if (!getProtocolConfig(
        scheme,
        config
    )) {
        return {};
    }

    size_t hostStart =
        schemePosition + 3;

    size_t pathStart =
        origin.find(
            '/',
            hostStart
        );

    std::string hostAndPort;

    if (pathStart ==
        std::string::npos) {

        hostAndPort =
            origin.substr(hostStart);
    }
    else {

        hostAndPort =
            origin.substr(
                hostStart,
                pathStart - hostStart
            );
    }

    std::string username;
    std::string password;

    size_t credentialsPosition =
        hostAndPort.find('@');

    if (credentialsPosition !=
        std::string::npos) {

        std::string credentials =
            hostAndPort.substr(
                0,
                credentialsPosition
            );

        hostAndPort =
            hostAndPort.substr(
                credentialsPosition + 1
            );

        size_t separator =
            credentials.find(':');

        if (separator !=
            std::string::npos) {

            username =
                credentials.substr(
                    0,
                    separator
                );

            password =
                credentials.substr(
                    separator + 1
                );
        }
        else {

            username = credentials;
        }
    }

    std::string host;

    int port =
        config.port;

    size_t portPosition =
        hostAndPort.find(':');

    if (portPosition !=
        std::string::npos) {

        host =
            hostAndPort.substr(
                0,
                portPosition
            );

        try {

            port =
                std::stoi(
                    hostAndPort.substr(
                        portPosition + 1
                    )
                );
        }
        catch (...) {

            return {};
        }

        if (port < 1 ||
            port > 65535) {

            return {};
        }
    }
    else {

        host =
            hostAndPort;
    }

    if (host.empty()) {
        return {};
    }

    std::string path =
        request.getPath();

    if (path.empty()) {
        path = "/";
    }

    return transport.send(
        scheme,
        host,
        port,
        username,
        password,
        request.getMethod(),
        path,
        request.getBody(),
        request.getSender(),
        request.getRecipient() 

    );
}