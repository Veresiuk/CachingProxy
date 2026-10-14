#include "curl_transport.h"

#include "protocols/http_transport.h"
#include "protocols/ftp_transport.h"
#include "protocols/smtp_transport.h"

#include <curl/curl.h>

#include <string>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iostream>

namespace {

    struct ResponseData {
        std::string body;
        std::string headers;
};

    struct UploadData {
        std::string message;
        std::size_t position = 0;
};

size_t writeBodyCallback(
    char* data,
    size_t size,
    size_t count,
    void* userData
) {
    const size_t totalSize = size * count;

    auto* response = static_cast<ResponseData*>(userData);
    response->body.append(data, totalSize);

    return totalSize;
}

size_t writeHeaderCallback(
    char* data,
    size_t size,
    size_t count,
    void* userData
) {
    const size_t totalSize = size * count;

    auto* response = static_cast<ResponseData*>(userData);
    response->headers.append(data, totalSize);

    return totalSize;
}

size_t readBodyCallback(
    char* buffer,
    size_t size,
    size_t count,
    void* userData
) {
    auto* upload = static_cast<UploadData*>(userData);

    const size_t bufferSize = size * count;
    const size_t remaining =
        upload->message.size() - upload->position;

    const size_t bytesToCopy =
        (std::min)(bufferSize, remaining);

    if (bytesToCopy > 0) {
        std::copy_n(
            upload->message.data() + upload->position,
            bytesToCopy,
            buffer
        );

        upload->position += bytesToCopy;
    }

    return bytesToCopy;
}

    std::string buildUrl(
        const std::string& scheme,
        const std::string& host,
        int port,
            const std::string& path
    ) {

    std::string url = scheme + "://" + host;

    if (port > 0) {
        url += ":" + std::to_string(port);
    }

    if (scheme == "http" || scheme == "https") {
        if (path.empty()) {
            url += "/";
        } else if (path.front() == '/') {
            url += path;
        } else {
            url += "/" + path;
        }
    } else if (scheme == "ftp") {
        if (!path.empty()) {
            if (path.front() == '/') {
                url += path;
            } else {
                url += "/" + path;
            }
        }
    }

    return url;
}

bool isHttpProtocol(const std::string& scheme) {
    return scheme == "http" || scheme == "https";
}

bool isSmtpProtocol(const std::string& scheme) {
    return scheme == "smtp" || scheme == "smtps";
}

}

HttpResponse CurlTransport::send(
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
) {
    static const CURLcode globalInitResult =
        curl_global_init(CURL_GLOBAL_DEFAULT);

    HttpResponse result;

    if (globalInitResult != CURLE_OK) {
        return result;
    }

    CURL* curl = curl_easy_init();

    if (curl == nullptr) {
        return result;
    }

    ResponseData response;
    UploadData upload;

    curl_slist* recipients = nullptr;

    const std::string url = buildUrl(
        scheme,
        host,
        port,
        path
    );

    curl_easy_setopt(
        curl,
        CURLOPT_URL,
        url.c_str()
    );

    curl_easy_setopt(
        curl,
        CURLOPT_FOLLOWLOCATION,
        1L
    );

    curl_easy_setopt(
        curl,
        CURLOPT_CONNECTTIMEOUT,
        15L
    );

    curl_easy_setopt(
        curl,
        CURLOPT_TIMEOUT,
        60L
    );

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEFUNCTION,
        writeBodyCallback
    );

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEDATA,
        &response
    );

    curl_easy_setopt(
        curl,
        CURLOPT_HEADERFUNCTION,
        writeHeaderCallback
    );

    curl_easy_setopt(
        curl,
        CURLOPT_HEADERDATA,
        &response
    );

    if (!username.empty()) {
        curl_easy_setopt(
            curl,
            CURLOPT_USERNAME,
            username.c_str()
        );
    }

    if (!password.empty()) {
        curl_easy_setopt(
            curl,
            CURLOPT_PASSWORD,
            password.c_str()
        );
    }

    if (scheme == "http") {
        configureHttp(curl, method, body);
    } else if (scheme == "https") {
        configureHttps(curl, method, body);
    } else if (scheme == "ftp") {
        configureFtp(curl, method, path);
    } else if (scheme == "smtp") {
        configureSmtp(
            curl,
            sender,
            recipient,
            &recipients
        );
    } else if (scheme == "smtps") {
        configureSmtps(
            curl,
            sender,
            recipient,
            &recipients
        );
    } else {
        curl_easy_cleanup(curl);
        return result;
    }

    if (isSmtpProtocol(scheme)) {
        upload.message =
            "To: " + recipient + "\r\n"
            "From: " + sender + "\r\n"
            "Subject: Caching Proxy\r\n"
            "\r\n" +
            body +
            "\r\n";

        curl_easy_setopt(
            curl,
            CURLOPT_UPLOAD,
            1L
        );

        curl_easy_setopt(
            curl,
            CURLOPT_READFUNCTION,
            readBodyCallback
        );

        curl_easy_setopt(
            curl,
            CURLOPT_READDATA,
            &upload
        );

        curl_easy_setopt(
            curl,
            CURLOPT_INFILESIZE_LARGE,
            static_cast<curl_off_t>(
                upload.message.size()
            )
        );
    }

    const CURLcode curlResult = curl_easy_perform(curl);


if (curlResult == CURLE_OK) {
    long responseCode = 0;

    curl_easy_getinfo(
        curl,
        CURLINFO_RESPONSE_CODE,
        &responseCode
    );

    result.statusCode =
        static_cast<int>(responseCode);

    result.body = std::move(response.body);

    if (isHttpProtocol(scheme)) {
        result.headers = normalizeHttpHeaders(
            response.headers,
            result.body.size()
        );
    } else if (
        scheme == "ftp" ||
        scheme == "smtp" ||
        scheme == "smtps"
    ) {
        result.statusCode = 200;
        result.headers.clear();
    } else {
        result.headers = std::move(response.headers);
    }
} else {
    std::cerr << "[CURL] Error: "
              << curl_easy_strerror(curlResult)
              << '\n';
}

curl_slist_free_all(recipients);
curl_easy_cleanup(curl);

return result;
}