#include "http_server.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <sstream>
#include <string>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

HttpServer::HttpServer(int port, Cache& cache, HttpClient& client) : port(port), cache(cache), client(client) {

}

void HttpServer::clearCache() {
    cache.clear();

    std::cout << "[CACHE] Cache cleared" << std::endl;
}

void HttpServer::stop() {
    running = false;
}

void HttpServer::start() {

    WSADATA wsaData;

    int result = WSAStartup(

        MAKEWORD(2, 2),
        &wsaData
    );

    if (result != 0) {
        
        std::cout << "WSAStartup failed" << std::endl;
        return;
    }

    SOCKET serverSocket = socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP
    );

    if (serverSocket == INVALID_SOCKET) {

        std::cout << "Failed to create socket" << std::endl;

        WSACleanup();
        return;

    }

    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    if(bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR) {

        std::cout << "Failed to bind socket" << std::endl;

        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {

        std::cout << "Failed to listen on socket" << std::endl;

        closesocket(serverSocket);
        WSACleanup();
        return;

    }

    std::cout << "\n========================================" << std::endl;
    std::cout << "        CACHING PROXY SERVER" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Server started on port " << port << std::endl;
    std::cout << "Waiting for requests..." << std::endl;

    while (running) {

    fd_set readSet;

    FD_ZERO(&readSet);
    FD_SET(serverSocket, &readSet);

    timeval timeout{};
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    int selectResult = select (
        0,
        &readSet,
        nullptr,
        nullptr,
        &timeout
    );

    if (selectResult == SOCKET_ERROR) {

        std::cout << "Select failed" << std::endl;
        break;

    }

    if (selectResult == 0) {

        continue;
    }
    
    SOCKET clientSocket = accept(

        serverSocket, 
        nullptr,
        nullptr

    );

    if (clientSocket == INVALID_SOCKET) {

        std::cout << "Failed to accept connection" << std::endl;
        break;

    }

    std::cout << "Client connected successfully" << std::endl;

    char buffer[4096];

    int bytesReceived = recv(

        clientSocket,
        buffer,
        sizeof(buffer) - 1,
        0

    );

    if (bytesReceived == SOCKET_ERROR) {

        std::cout << "Failed to receive request" << std::endl;

        closesocket(clientSocket);
        continue;

    }

    if (bytesReceived == 0) {

        std::cout <<  "Client disconnected" << std::endl;

        closesocket(clientSocket);
        continue;

    }

    buffer [bytesReceived] = '\0';

    std::string rawRequest(buffer);

    std::cout << "Received request:" << std::endl;
    std::cout << rawRequest << std::endl;
    
    std::istringstream requestStream(rawRequest);

    std::string method;
    std::string path;
    std::string version;

    requestStream >> method >> path >> version;

    std::cout << "Method: " << method << std::endl;
    std::cout << "Path: " << path << std::endl;
    std::cout << "Version: " << version << std::endl;

    size_t headerEnd = rawRequest.find("\r\n\r\n");

    std::string headers;
    std::string body;

    if (headerEnd != std::string::npos) {

        headers = rawRequest.substr(
            0,
            headerEnd
        );

        body = rawRequest.substr(
            headerEnd + 4
        );
    }

    std::string sender;
    std::string senderHeader = "X-Sender:";

    size_t senderPosition = headers.find(senderHeader);

    if (senderPosition != std::string::npos) {

        size_t valueStart = senderPosition + senderHeader.size();

        while (
            valueStart < headers.size() && headers[valueStart] == ' '
        ) {
            valueStart++;
        }

        size_t valueEnd = headers.find("\r\n", valueStart);

        sender = headers.substr(valueStart, valueEnd - valueStart);

    }

    std::string recipient;
    std::string recipientHeader = "X-Recipient:";

    size_t recipientPosition = headers.find(recipientHeader);

    if(recipientPosition != std::string::npos) {

        size_t valueStart = recipientPosition + recipientHeader.size();

        while (
            valueStart < headers.size() && headers[valueStart] == ' '
        ) {
            valueStart++;
        }

        size_t valueEnd = headers.find("\r\n", valueStart);

        recipient = headers.substr(valueStart, valueEnd - valueStart);
    }

    std::cout << "Sender: " << sender << std::endl;
    std::cout << "Recipient: " << recipient << std::endl;
    std::cout << "Body: " << body << std::endl;

    Request request(
        method,
        path,
        body,
        sender,
        recipient
    );

    std::cout << "\n[REQUEST] " << request.getMethod() << " " << request.getPath() << std::endl;

    bool cacheable =
        client.isCacheable(request);
    
    if (cacheable && cache.contains(path)) {

        std::cout << "[CACHE]    HIT" << std::endl;

        std::string cachedResponse = cache.get(path);

        size_t cacheHeaderPosition = cachedResponse.find("X-Cache: MISS");

        if (cacheHeaderPosition != std::string::npos) {

            cachedResponse.replace(
                cacheHeaderPosition,
                std::string("X-Cache: MISS").size(),
                "X-Cache: HIT"
            );
        }

        send(

            clientSocket,
            cachedResponse.c_str(),
            static_cast<int>(cachedResponse.size()),
            0

        );

        std::cout << "[CLIENT]   Cached response sent" << std::endl;

        closesocket(clientSocket);

        continue;
    }

    if (cacheable) {

        std::cout << "[CACHE]    MISS" << std::endl;

    }
    else {

        std::cout << "[CACHE]    BYPASS" << std::endl;

    }
    

    HttpResponse response = client.sendRequest(request);

    if (response.statusCode == 0) {
        std::string errorResponse =
            "HTTP/1.1 502 Bad Gateway\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n";

        send(
            clientSocket,
            errorResponse.c_str(),
            static_cast<int>(errorResponse.size()),
            0
        );

        closesocket(clientSocket);

        continue;
    }

    std::cout << "[ORIGIN]   Response received" << std::endl;

    if (response.statusCode == 0) {

        std::cout << "Failed to get response from origin" << std::endl;

        closesocket(clientSocket);
        continue;
    }


    std::string statusText;

    if (response.statusCode == 200) {
        statusText = "OK";
    }
    else if (response.statusCode == 201) {
        statusText = "Created";
    }
    else if (response.statusCode == 400) {
        statusText = "Bad Request";
    }
    else if (response.statusCode == 404) {
        statusText = "Not Found";
    }
    else if (response.statusCode == 500) {
        statusText = "Internal Server Error";
    }
    else {
        statusText = "Unknown";
    }

    std::cout << "[RESPONSE] " << response.statusCode << " " << statusText << std::endl;

    std::string responseHeaders = response.headers;

    size_t firstLineEnd = responseHeaders.find("\r\n");

    if (firstLineEnd != std::string::npos) {

        responseHeaders = responseHeaders.substr(firstLineEnd + 2);
    }

    size_t transferEncodingPosition = responseHeaders.find("Transfer-Encoding: chunked\r\n");

    if (transferEncodingPosition != std::string::npos) {

        responseHeaders.erase(
            transferEncodingPosition,
            std::string("Transfer-Encoding: chunked\r\n").size());
    }

    while (
        responseHeaders.size() >= 2 &&
        responseHeaders.compare(
            responseHeaders.size() - 2,
            2,
            "\r\n"
        ) == 0
    ) {
        responseHeaders.erase(
            responseHeaders.size() - 2
        );
    }

    responseHeaders += "Content-Length: " + std::to_string(response.body.size()) + "\r\n";

    std::string httpResponse =
        "HTTP/1.1 " +
        std::to_string(response.statusCode) +
        " " +
        statusText +
        "\r\n" +
        responseHeaders;

    if (cacheable) {

        httpResponse += "X-Cache: MISS\r\n";

    }

    httpResponse +=
        "\r\n" + response.body;

    if (cacheable) {

        cache.set(path, httpResponse);

        std::cout << "[CACHE]    Response saved" << std::endl;
}

    size_t totalSent = 0;

    while (totalSent < httpResponse.size()) {

        int bytesSent = send(
            clientSocket,
            httpResponse.c_str() + totalSent,
            static_cast<int>(httpResponse.size() - totalSent),
            0
        );

        if (bytesSent == SOCKET_ERROR) {

            std::cout << "Failed to send response" << std::endl;

            break;
        }

        totalSent += bytesSent;
    }

    if (totalSent == httpResponse.size()) {

        std::cout << "[CLIENT]   Response sent" << std::endl;
    }

    closesocket(clientSocket);

    }
    
    closesocket(serverSocket);
    WSACleanup();

}
