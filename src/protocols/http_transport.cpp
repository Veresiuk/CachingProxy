#include "protocols/http_transport.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>

namespace {

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");

    if (first == std::string::npos) {
        return "";
    }

    const auto last = value.find_last_not_of(" \t\r\n");

    return value.substr(first, last - first + 1);
}

std::string toLower(std::string value) {
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    return value;
}

bool startsWith(
    const std::string& value,
    const std::string& prefix
) {
    return value.size() >= prefix.size()
        && value.compare(0, prefix.size(), prefix) == 0;
}

}

void configureHttp(
    CURL* curl,
    const std::string& method,
    const std::string& body
) {
    curl_easy_setopt(
        curl,
        CURLOPT_CUSTOMREQUEST,
        method.c_str()
    );

    if (!body.empty()) {
        curl_easy_setopt(
            curl,
            CURLOPT_POSTFIELDS,
            body.c_str()
        );

        curl_easy_setopt(
            curl,
            CURLOPT_POSTFIELDSIZE,
            static_cast<long>(body.size())
        );
    }
}

void configureHttps(
    CURL* curl,
    const std::string& method,
    const std::string& body
) {
    configureHttp(curl, method, body);

    curl_easy_setopt(
        curl,
        CURLOPT_SSL_VERIFYPEER,
        1L
    );

    curl_easy_setopt(
        curl,
        CURLOPT_SSL_VERIFYHOST,
        2L
    );
}

std::string normalizeHttpHeaders(
    const std::string& rawHeaders,
    std::size_t bodySize
) {
    std::istringstream stream(rawHeaders);
    std::string line;
    std::string result;
    bool hasContentLength = false;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line.empty()) {
            continue;
        }

        if (startsWith(line, "HTTP/")) {
            result.clear();
            hasContentLength = false;
            continue;
        }

        const auto separator = line.find(':');

        if (separator == std::string::npos) {
            continue;
        }

        const std::string name = toLower(
            trim(line.substr(0, separator))
        );

        if (name == "transfer-encoding") {
            continue;
        }

        if (name == "content-length") {
            hasContentLength = true;
            continue;
        }

        result += line + "\r\n";
    }

    if (!hasContentLength) {
        result += "Content-Length: "
            + std::to_string(bodySize)
            + "\r\n";
    }

    return result;
}