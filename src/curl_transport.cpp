#include "curl_transport.h"

#include <curl/curl.h>

#include <iostream>
#include <string>
#include <algorithm>

namespace {

// Зберігає тіло відповіді.
size_t writeBodyCallback(
    char* data,
    size_t size,
    size_t count,
    void* userData
) {
    size_t totalSize = size * count;

    auto* responseBody =
        static_cast<std::string*>(userData);

    responseBody->append(
        data,
        totalSize
    );

    return totalSize;
}

// Зберігає заголовки відповіді.
size_t writeHeaderCallback(
    char* data,
    size_t size,
    size_t count,
    void* userData
) {
    size_t totalSize = size * count;

    auto* responseHeaders =
        static_cast<std::string*>(userData);

    responseHeaders->append(
        data,
        totalSize
    );

    return totalSize;
}

// Передає дані SMTP.
size_t readBodyCallback(
    char* buffer,
    size_t size,
    size_t count,
    void* userData
) {
    auto* data =
        static_cast<std::string*>(userData);

    size_t bufferSize =
        size * count;

    if (data->empty()) {
        return 0;
    }

    size_t bytesToCopy =
        data->size() < bufferSize
            ? data->size()
            : bufferSize;

    std::copy(
        data->begin(),
        data->begin() + bytesToCopy,
        buffer
    );

    data->erase(
        0,
        bytesToCopy
    );

    return bytesToCopy;
}

// Перевіряє HTTP.
bool isHttp(
    const std::string& scheme
) {
    return scheme == "http";
}

// Перевіряє HTTPS.
bool isHttps(
    const std::string& scheme
) {
    return scheme == "https";
}

// Перевіряє FTP.
bool isFtp(
    const std::string& scheme
) {
    return scheme == "ftp";
}

// Перевіряє SMTP.
bool isSmtp(
    const std::string& scheme
) {
    return scheme == "smtp";
}

// Перевіряє SMTPS.
bool isSmtps(
    const std::string& scheme
) {
    return scheme == "smtps";
}

// Налаштовує спільні параметри.
void configureCommonOptions(
    CURL* curl,
    const std::string& url,
    std::string& responseBody,
    std::string& responseHeaders
) {
    curl_easy_setopt(
        curl,
        CURLOPT_URL,
        url.c_str()
    );

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEFUNCTION,
        writeBodyCallback
    );

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEDATA,
        &responseBody
    );

    curl_easy_setopt(
        curl,
        CURLOPT_HEADERFUNCTION,
        writeHeaderCallback
    );

    curl_easy_setopt(
        curl,
        CURLOPT_HEADERDATA,
        &responseHeaders
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
}

// Налаштовує авторизацію.
void configureCredentials(
    CURL* curl,
    const std::string& username,
    const std::string& password
) {
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
}

// Налаштовує HTTP.
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
    }
}

// Налаштовує HTTPS.
void configureHttps(
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
    }

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

// Налаштовує FTP.
void configureFtp(
    CURL* curl
) {
    curl_easy_setopt(
        curl,
        CURLOPT_DIRLISTONLY,
        0L
    );
}

// Налаштовує SMTP.
void configureSmtp(
    CURL* curl,
    const std::string& sender,
    const std::string& recipient,
    std::string& message,
    struct curl_slist*& recipients
) {
    if (!sender.empty()) {

        curl_easy_setopt(
            curl,
            CURLOPT_MAIL_FROM,
            sender.c_str()
        );
    }

    if (!recipient.empty()) {

        recipients =
            curl_slist_append(
                recipients,
                recipient.c_str()
            );

        curl_easy_setopt(
            curl,
            CURLOPT_MAIL_RCPT,
            recipients
        );
    }

    curl_easy_setopt(
        curl,
        CURLOPT_READFUNCTION,
        readBodyCallback
    );

    curl_easy_setopt(
        curl,
        CURLOPT_READDATA,
        &message
    );

    curl_easy_setopt(
        curl,
        CURLOPT_UPLOAD,
        1L
    );

    curl_easy_setopt(
        curl,
        CURLOPT_INFILESIZE_LARGE,
        static_cast<curl_off_t>(
            message.size()
        )
    );
}

// Налаштовує SMTPS.
void configureSmtps(
    CURL* curl
) {
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

} // namespace

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
    HttpResponse response;

    CURL* curl =
        curl_easy_init();

    if (curl == nullptr) {

        std::cout
            << "[CURL] Failed to initialize libcurl"
            << std::endl;

        return response;
    }

    std::string url =
        scheme +
        "://" +
        host +
        ":" +
        std::to_string(port);

    if (!path.empty()) {
        url += path;
    }

    std::string responseBody;
    std::string responseHeaders;

    configureCommonOptions(
        curl,
        url,
        responseBody,
        responseHeaders
    );

    configureCredentials(
        curl,
        username,
        password
    );

    // Список SMTP отримувачів.
    struct curl_slist* recipients =
        nullptr;

    // Дані SMTP повідомлення.
    std::string message;

    // HTTP.
    if (isHttp(scheme)) {

        configureHttp(
            curl,
            method,
            body
        );
    }

    // HTTPS.
    else if (isHttps(scheme)) {

        configureHttps(
            curl,
            method,
            body
        );
    }

    // FTP.
    else if (isFtp(scheme)) {

        configureFtp(
            curl
        );
    }

    // SMTP.
    else if (isSmtp(scheme)) {

        message =
            "From: " +
            sender +
            "\r\n"
            "To: " +
            recipient +
            "\r\n"
            "Subject: Caching Proxy\r\n"
            "\r\n" +
            body +
            "\r\n";

        configureSmtp(
            curl,
            sender,
            recipient,
            message,
            recipients
        );

        std::cout
            << "[CURL] SMTP connection configured"
            << std::endl;
    }

    // SMTPS.
    else if (isSmtps(scheme)) {

        message =
            "From: " +
            sender +
            "\r\n"
            "To: " +
            recipient +
            "\r\n"
            "Subject: Caching Proxy\r\n"
            "\r\n" +
            body +
            "\r\n";

        configureSmtp(
            curl,
            sender,
            recipient,
            message,
            recipients
        );

        configureSmtps(
            curl
        );

        std::cout
            << "[CURL] SMTPS connection configured"
            << std::endl;
    }

    // Протокол не підтримується.
    else {

        std::cout
            << "[CURL] Unsupported protocol: "
            << scheme
            << std::endl;

        curl_easy_cleanup(curl);

        return {};
    }

    CURLcode result =
        curl_easy_perform(curl);

    // Звільняємо список отримувачів.
    if (recipients != nullptr) {

        curl_slist_free_all(
            recipients
        );
    }

    if (result != CURLE_OK) {

        std::cout
            << "[CURL] Protocol: "
            << scheme
            << std::endl;

        std::cout
            << "[CURL] Error code: "
            << static_cast<int>(result)
            << std::endl;

        std::cout
            << "[CURL] Error: "
            << curl_easy_strerror(result)
            << std::endl;

        curl_easy_cleanup(curl);

        return {};
    }

    long statusCode = 0;

    curl_easy_getinfo(
        curl,
        CURLINFO_RESPONSE_CODE,
        &statusCode
    );

   response.statusCode = static_cast<int>(statusCode);

    response.headers = responseHeaders;

    response.body = responseBody;

    std::cout << "[CURL] Response body: " << response.body << std::endl;
    std::cout << "[CURL] Response headers:" << std::endl;
    std::cout << response.headers << std::endl;

    // FTP не має HTTP status code.
    if (isFtp(scheme)) {

        response.statusCode = 200;

        response.headers.clear();
    }

    // SMTP використовує власні коди,
    // тому для Proxy повертаємо HTTP 200.
    if (isSmtp(scheme) ||
        isSmtps(scheme)) {

        response.statusCode = 200;

        response.headers.clear();
    }

    std::cout
        << "[CURL] Protocol: "
        << scheme
        << std::endl;

    std::cout
        << "[CURL] Status: "
        << response.statusCode
        << std::endl;

    std::cout
        << "[CURL] Response size: "
        << response.body.size()
        << " bytes"
        << std::endl;

    curl_easy_cleanup(curl);

    return response;
}