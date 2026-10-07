#include <curl/curl.h>
#include <iostream>

int main() {

    curl_version_info_data* info =
        curl_version_info(CURLVERSION_NOW);

    std::cout << "libcurl version: "
              << info->version
              << std::endl;

    std::cout << "SSL backend: "
              << (info->ssl_version ? info->ssl_version : "none")
              << std::endl;

    std::cout << "Protocols:" << std::endl;

    for (const char* const* protocol = info->protocols;
         *protocol != nullptr;
         ++protocol) {

        std::cout << "  "
                  << *protocol
                  << std::endl;
    }

    return 0;
}