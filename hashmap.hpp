// File: hashmap.hpp
// Description: Header file for a simple in-memory file server map implemented using C++.
//              Provides functionality to store, retrieve, and access files in memory
//              using an unordered map. Suitable for lightweight file storage needs.
// Author: [Your Name]
// Date: [Insert Date]

#ifndef HASHMAP_HPP
#define HASHMAP_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_map>

// Class: FileServerMap
// Purpose: Represents a simple in-memory map for storing and retrieving files.
//          It uses an unordered_map to manage file entries, where each key-value
//          pair represents a file's name and its content (as a byte vector).
class FileServerMap {
public:
    // Method: insert
    // Purpose: Inserts or updates a file in the map.
    // Parameters:
    //   - key: The name of the file to insert or update.
    //   - data: The content of the file as a vector of bytes.
    // Returns:
    //   - true if the key already existed and was replaced.
    //   - false if a new key was added.
    bool insert(const std::string &key, const std::vector<uint8_t> &data);

    // Method: get
    // Purpose: Retrieves the file data associated with the given key.
    // Parameters:
    //   - key: The name of the file to retrieve.
    // Returns:
    //   - A vector of bytes representing the file's content.
    // Throws:
    //   - std::runtime_error if the key is not found in the map.
    std::vector<uint8_t> get(const std::string &key) const;

    // Method: entries
    // Purpose: Provides a const reference to the underlying map for inspecting or
    //          persisting all stored entries.
    // Returns:
    //   - A const reference to the unordered_map containing all file entries.
    const std::unordered_map<std::string, std::vector<uint8_t>>& entries() const {
        return map_;
    }

private:
    // Member: map_
    // Purpose: The core data storage for the file server map.
    //          Keys are file names (std::string), and values are the file contents
    //          as vectors of bytes (std::vector<uint8_t>).
    std::unordered_map<std::string, std::vector<uint8_t>> map_;
};

#endif // HASHMAP_HPP