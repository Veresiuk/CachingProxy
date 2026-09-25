#include "cache.h"

bool Cache::contains(const std::string& key) const {

    return cache.find(key) != cache.end();

}

std::string Cache::get (const std::string& key) {

    return cache.at(key);

}

void Cache::set (const std::string& key, const std::string& response) {

    cache[key] = response;

}

void Cache::clear() {

    cache.clear();

}
