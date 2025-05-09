// include/hashmap.hpp
#ifndef HASHMAP_HPP
#define HASHMAP_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_map>

// Simple in-memory map for storing files
class FileServerMap {
public:
    // Inserts or updates a file; returns true if key existed (replaced), false if new
    bool insert(const std::string &key, const std::vector<uint8_t> &data);

    // Retrieves file data; throws std::runtime_error if not found
    std::vector<uint8_t> get(const std::string &key) const;

    // Returns a reference to the underlying map for persistence
    const std::unordered_map<std::string, std::vector<uint8_t>>& entries() const {
        return map_;
    }

private:
    std::unordered_map<std::string, std::vector<uint8_t>> map_;
};

#endif // HASHMAP_HPP
