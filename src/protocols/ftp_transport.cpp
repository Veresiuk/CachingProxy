#include "protocols/ftp_transport.h"

void configureFtp(
    CURL* curl,
    const std::string& method,
    const std::string& path
) {
    if (method == "GET") {
        curl_easy_setopt(
            curl,
            CURLOPT_DIRLISTONLY,
            0L
        );
    }

    if (method == "PUT") {
        curl_easy_setopt(
            curl,
            CURLOPT_UPLOAD,
            1L
        );
    }

    (void)path;
}