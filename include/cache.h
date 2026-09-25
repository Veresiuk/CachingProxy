#pragma once

#include <string>
#include <unordered_map>


class Cache {
    private:

    std::unordered_map <std::string, std::string> cache;

    public:

    bool contains (const std::string& key) const;

    std::string get (const std::string& key);

    void set(const std::string& key, const std::string& response);

    void clear();

};