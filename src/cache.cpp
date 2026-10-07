#include "cache.h"

bool Cache::contains(const std::string& key) const {

    std::lock_guard<std::mutex> lock(mutex);

    return cache.find(key) != cache.end();

}

std::string Cache::get (const std::string& key) {

    std::lock_guard<std::mutex> lock(mutex);

    return cache.at(key);

}

void Cache::set (const std::string& key, const std::string& response) {

    std::lock_guard<std::mutex> lock(mutex);

    cache[key] = response;

}

void Cache::clear() {

    std::lock_guard<std::mutex> lock(mutex);

    cache.clear();

}
