#pragma once

#include <string>
#include "cache.h"
#include "http_client.h"

class HttpServer {

private:
    int port;
    Cache& cache;
    HttpClient& client;
public:
    HttpServer(int port, Cache& cache, HttpClient& client);

    void start();
    
};

