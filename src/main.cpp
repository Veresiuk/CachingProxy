#include "http_server.h"
#include "cache.h"
#include "http_client.h"

#include <iostream>
#include <string>
#include <thread>

int main(int argc, char* argv[]) {

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

    std::thread serverThread([&server]() {
        server.start();
    }
);

    std::string command;

    while (true) {

        std::getline(std::cin, command);

        if (command == "clear-cache") {

            server.clearCache();

        }
        else if (command == "help") {

            std::cout << "Available commands:" << std::endl;
            std::cout << "  clear-cache  - clear response cache" << std::endl;
            std::cout << "  help         - show available commands" << std::endl;
            std::cout << "  exit         - stop proxy server" << std::endl;

        }
        else if (command == "exit") {

            server.stop();

            break;

        }
        else if (!command.empty()) {

            std::cout << "Unknown command: " << command << std::endl;
            std::cout << "Type 'help' to see available commands." << std::endl;

        }
    }

    serverThread.join();

    return 0;
}