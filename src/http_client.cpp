#include "http_client.h"
#include <windows.h>
#include <winhttp.h>
#include <iostream>
#include <vector>

HttpClient::HttpClient(const std::string& origin) {

    this->origin = origin;

}

HttpResponse HttpClient::sendRequest(const Request& request) {

    std::string method = request.getMethod();
    std::string path = request.getPath();

    std::string url = origin + path;

    std::string scheme;
    std::string host;
    int port;

    size_t schemeEnd = origin.find("://");

    if (schemeEnd == std::string::npos) {

    std::cout << "Invalid origin URL" << std::endl;
    return {0, "", ""};

    }

    scheme = origin.substr(0, schemeEnd);
    host = origin.substr(schemeEnd + 3);

    std::cout << "Scheme: " << scheme << std::endl;
    std::cout << "Origin: " << origin << std::endl;
    std::cout << "Path: " << path << std::endl;
    std::cout << "URL: " << url << std::endl;
    std::cout << "Host: " << host << std::endl;

    DWORD flags = 0;

    if (scheme == "http") {

        port = 80;

    }
    else if (scheme == "https") {

        port = 443;
        flags = WINHTTP_FLAG_SECURE;

    }
    else {

        std::cout << "Unsupported protocol" << std::endl;
        return {0, "", ""};
    }

    std::cout << "Port: " << port << std::endl;

    HINTERNET session = WinHttpOpen(

    L"CachingProxy",
    0,
    0,
    0,
    0

    );

    if (!session) {

        std::cout << "Failed to open WinHTTP session" << std::endl;
        return {0, "", ""};
    }

    std::wstring wideHost(host.begin(), host.end());
    std::wstring wideMethod(method.begin(), method.end());
    std::wstring widePath(path.begin(), path.end());

    HINTERNET connection = WinHttpConnect(

    session,
    wideHost.c_str(),
    port,
    0

    );

    if (!connection) {

        std::cout << "Failed to connect to server" << std::endl;
        return {0, "", ""};

    }

    HINTERNET requestHandle = WinHttpOpenRequest(

        connection,
        wideMethod.c_str(),
        widePath.c_str(),
        nullptr,
        nullptr,
        nullptr,
        flags

    );

    if (!requestHandle) {

        std::cout << "Failed to create HTTP request" << std::endl;
        return {0, "", ""};
        
    }

    BOOL result = WinHttpSendRequest(

        requestHandle,
        nullptr,
        0,
        nullptr,
        0,
        0,
        0

    );

    if (!result) {

        std::cout << "Failed to send request" << std::endl;
        std::cout << "Error code: " << GetLastError() << std::endl;
        return {0, "", ""};

    }


    BOOL response = WinHttpReceiveResponse(

        requestHandle,
        nullptr

    );

    if (!response) {

        std::cout << "Failed to receive response" << std::endl;
        std::cout << "Error code: " << GetLastError() << std::endl;
        return {0, "", ""};
    }

    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);

    BOOL headerResult = WinHttpQueryHeaders(

      requestHandle,
      WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
      nullptr,
      &statusCode,
      &statusCodeSize,
      nullptr

    );

    if (headerResult == FALSE) {

        std::cout << "Failed to get status code" << std::endl;
        std::cout << "Error code: " << GetLastError() << std::endl;

        WinHttpCloseHandle(requestHandle);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);

        return {0, "", ""};
    }

    std::cout << "Status code: " << statusCode << std::endl;

    DWORD headersSize = 0;

    BOOL headersResult = WinHttpQueryHeaders(

        requestHandle,
        WINHTTP_QUERY_RAW_HEADERS_CRLF,
        nullptr,
        nullptr,
        &headersSize,
        nullptr

    );

    if (headersResult == FALSE) {

        DWORD error = GetLastError();

        if (error != ERROR_INSUFFICIENT_BUFFER) {

            std::cout << "Failed to get response headers" << std::endl;
            std::cout << "Error code: " << error << std::endl;

            WinHttpCloseHandle(requestHandle);
            WinHttpCloseHandle(connection);
            WinHttpCloseHandle(session);

            return {0, "", ""};

        }

    }

    DWORD headersCount = headersSize / sizeof(wchar_t) + 1;

    std::vector<wchar_t> headersBuffer(headersCount);

    headersSize = headersCount * sizeof(wchar_t);

    BOOL headersRead = WinHttpQueryHeaders(

        requestHandle,
        WINHTTP_QUERY_RAW_HEADERS_CRLF,
        nullptr,
        headersBuffer.data(),
        &headersSize,
        nullptr

    );

    if (!headersRead) {

        std::cout << "Failed to read response headers" << std::endl;
        std::cout << "Error code: " << GetLastError() << std::endl;
        
        WinHttpCloseHandle(requestHandle);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(session);

        return {0, "", ""};
        
    }

    std::wstring wideHeaders(headersBuffer.data());

    int headersLength = WideCharToMultiByte(
        CP_UTF8,
        0,
        wideHeaders.c_str(),
        -1,
        nullptr,
        0,
        nullptr,
        nullptr
    );

    std::string responseHeaders(headersLength - 1, '\0');

    WideCharToMultiByte(
        CP_UTF8,
        0,
        wideHeaders.c_str(),
        -1,
        responseHeaders.data(),
        headersLength,
        nullptr,
        nullptr
    );

    std::cout << "Response headers:" << std::endl;
    std::cout << responseHeaders << std::endl;

    std::cout << "Response received successfully!" << std::endl;

    DWORD bytesAvailable = 0;

    BOOL dataAvailable = WinHttpQueryDataAvailable(

        requestHandle,
        &bytesAvailable

    );

    if (!dataAvailable) {

        std::cout << "Failed to query response data" << std::endl;
        return {0, "", ""};

    }

    std::string responseBody;

    while (bytesAvailable > 0) {
    std::vector<char> buffer(bytesAvailable);
    DWORD bytesRead = 0;

    BOOL dataRead = WinHttpReadData(
        requestHandle,
        buffer.data(),
        bytesAvailable,
        &bytesRead
    );

    if(!dataRead) {

        std::cout << "Failed to read response data" << std::endl;
        return {0, "", ""};

    }

    responseBody.append(buffer.begin(), buffer.begin() + bytesRead);

    if (!WinHttpQueryDataAvailable(
        requestHandle,
        &bytesAvailable
    )) {

        std::cout << "Failed to query response data" << std::endl;
        return {0, "", ""};

    }

    }

    std::cout << responseBody << std::endl;

    std::cout << "Bytes available: " << bytesAvailable << std::endl;

    WinHttpCloseHandle(requestHandle);
    WinHttpCloseHandle(connection);
    WinHttpCloseHandle(session);

    return {
        static_cast<int>(statusCode),
        responseHeaders,
        responseBody
    };

}

