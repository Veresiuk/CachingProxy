#include "protocols/smtp_transport.h"

#include <cstdlib>

void configureSmtp(
    CURL* curl,
    const std::string& sender,
    const std::string& recipient,
    curl_slist** recipients
) {
    curl_easy_setopt(
        curl,
        CURLOPT_MAIL_FROM,
        sender.c_str()
    );

    *recipients = curl_slist_append(
        *recipients,
        recipient.c_str()
    );

    curl_easy_setopt(
        curl,
        CURLOPT_MAIL_RCPT,
        *recipients
    );
}

void configureSmtps(
    CURL* curl,
    const std::string& sender,
    const std::string& recipient,
    curl_slist** recipients
) {
    configureSmtp(
        curl,
        sender,
        recipient,
        recipients
    );

    curl_easy_setopt(
        curl,
        CURLOPT_USE_SSL,
        static_cast<long>(CURLUSESSL_ALL)
    );

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

    char* caBundle = nullptr;
    size_t bufferSize = 0;

    if (_dupenv_s(&caBundle, &bufferSize, "CURL_CA_BUNDLE") == 0
        && caBundle != nullptr) {
        if (caBundle[0] != '\0') {
            curl_easy_setopt(curl, CURLOPT_CAINFO, caBundle);
    }

        free(caBundle);
    }
}
