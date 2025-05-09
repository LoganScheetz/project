#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <string>
#include <vector>
#include <stdexcept>

using Bytes = std::vector<uint8_t>;

namespace Protocol {

// File message structure
struct File {
    std::string name;   // File name
    Bytes data;         // File data (byte array)

    // Serialize the File message
    Bytes serialize() const;

    // Deserialize a File message
    static File deserialize(const Bytes& bytes);
};

// Request message structure
struct Request {
    std::string name;   // File name being requested

    // Serialize the Request message
    Bytes serialize() const;

    // Deserialize a Request message
    static Request deserialize(const Bytes& bytes);
};

// Status message structure
struct Status {
    std::string message;  // Status message (e.g., success, error)

    // Serialize the Status message
    Bytes serialize() const;

    // Deserialize a Status message
    static Status deserialize(const Bytes& bytes);
};

// XOR encryption/decryption
Bytes xorEncryptDecrypt(const Bytes& input, uint8_t key = 42);

} // namespace Protocol

#endif // PROTOCOL_HPP