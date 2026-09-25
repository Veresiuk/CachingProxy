#include "http_server.h"
#include "cache.h"
#include "http_client.h"

#include <iostream>
#include <string>
#include <fstream>

int main(int argc, char* argv[]) {

    if (argc == 2 && std::string(argv[1]) == "--clear-cache") {

    std::ofstream clearFile("clear_cache.flag");

    if (!clearFile) {

        std::cout << "Failed to clear cache" << std::endl;

        return 1;
    }

    clearFile.close();

    std::cout << "Cache clear requested successfully" << std::endl;

    return 0;
    }

    if (argc != 5) {

        std::cout << "Usage: caching-proxy --port <number> --origin <url>" << std::endl;
        std::cout << "Or: caching-proxy --clear-cache" << std::endl;

        return 1;
    }

    if (std::string(argv[1]) != "--port" || std::string(argv[3]) != "--origin") {

        std::cout << "Invalid arguments" << std::endl;

        return 1;
    }

    int port;

    try {

        port = std::stoi(argv[2]);

    }
    catch (...) {

        std::cout << "Invalid port number" << std::endl;

        return 1;
    }

    std::string origin = argv[4];

    std::cout << "Starting Caching Proxy..." << std::endl;
    std::cout << "Port: " << port << std::endl;
    std::cout << "Origin: " << origin << std::endl;

    Cache cache;

    HttpClient client(origin);

    HttpServer server(
        port,
        cache,
        client
    );

    server.start();

    return 0;
}