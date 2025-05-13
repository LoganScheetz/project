// File: hashmap.cpp
// Description: Implementation of the FileServerMap class for managing an in-memory
//              file storage using a hashmap. Provides methods to insert, update,
//              and retrieve file data.
// Author: Logan Scheetz
// Date: 5/12/25

#include "hashmap.hpp"
#include "protocol.hpp"

// Method: insert
// Purpose: Inserts or updates a file in the map.
// Parameters:
//   - key: The name of the file to insert or update.
//   - data: The content of the file as a vector of bytes.
// Returns:
//   - true if the key already existed and the file was replaced.
//   - false if the key is new and the file was added.
bool FileServerMap::insert(const std::string &key, const std::vector<uint8_t> &data) {
    auto it = map_.find(key);                // Search for the key in the map
    bool existed = (it != map_.end());       // Check if the key already exists
    map_[key] = data;                        // Insert or update the file data
    return existed;                          // Return whether the key existed
}

// Method: get
// Purpose: Retrieves the file data associated with the given key.
// Parameters:
//   - key: The name of the file to retrieve.
// Returns:
//   - A vector of bytes representing the file's content.
// Throws:
//   - std::runtime_error if the key is not found in the map.
std::vector<uint8_t> FileServerMap::get(const std::string &key) const {
    auto it = map_.find(key);                // Search for the key in the map
    if (it == map_.end()) {                  // If the key is not found
        throw std::runtime_error("File not found: " + key); // Throw an exception
    }
    return it->second;                       // Return the file data
}