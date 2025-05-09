// src/hashmap.cpp
#include "hashmap.hpp"

// Inserts or updates a file; returns true if key existed (replaced), false if new
bool FileServerMap::insert(const std::string &key, const std::vector<uint8_t> &data) {
    auto it = map_.find(key);
    bool existed = (it != map_.end());
    map_[key] = data;
    return existed;
}

// Retrieves file data; throws std::runtime_error if not found
std::vector<uint8_t> FileServerMap::get(const std::string &key) const {
    auto it = map_.find(key);
    if (it == map_.end()) {
        throw std::runtime_error("File not found: " + key);
    }
    return it->second;
}
