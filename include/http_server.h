#pragma once

#include <string>
#include <atomic>
#include "cache.h"
#include "http_client.h"

class HttpServer {

private:
    int port;
    Cache& cache;
    HttpClient& client;
    std::atomic<bool> running = true;
public:
    HttpServer(int port, Cache& cache, HttpClient& client);

    void start();

    void clearCache();

    void stop();
    
};

