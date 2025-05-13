// File: protocol.hpp
// Description: Header file defining protocol structures and utilities for message
//              serialization, deserialization, and encryption/decryption. This includes
//              file transfer messages, request messages, and status messages.
// Author: [Your Name]
// Date: [Insert Date]

#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <string>
#include <vector>
#include <stdexcept>

// Type alias for byte buffer
using Bytes = std::vector<uint8_t>;

// Function: xor42
// Purpose: Encrypts or decrypts a byte buffer using XOR with a key (default: 42).
// Parameters:
//   - input: The byte buffer to be encrypted or decrypted.
//   - key: The key to use for the XOR operation (default is 42).
// Returns:
//   - A new byte buffer after applying the XOR operation.
Bytes xor42(const Bytes& input, uint8_t key = 42);

// Class: FileMessage
// Purpose: Represents a file message, containing a file's name and its data.
//          Provides serialization and deserialization methods.
class FileMessage {
public:
    std::string name; // Name of the file
    Bytes data;       // File content as a byte buffer

    // Constructor
    // Parameters:
    //   - n: Name of the file
    //   - d: File content as a byte buffer
    FileMessage(std::string n, Bytes d);

    // Method: serialize
    // Purpose: Serializes the FileMessage into a byte buffer.
    // Returns:
    //   - A byte buffer representing the serialized FileMessage.
    Bytes serialize() const;

    // Static Method: deserialize
    // Purpose: Deserializes a byte buffer into a FileMessage object.
    // Parameters:
    //   - bytes: The byte buffer to deserialize.
    // Returns:
    //   - A FileMessage object.
    static FileMessage deserialize(const Bytes& bytes);
};

// Class: RequestMessage
// Purpose: Represents a request message, containing the name of the requested file.
//          Provides serialization and deserialization methods.
class RequestMessage {
public:
    std::string name; // Name of the requested file

    // Constructor
    // Parameters:
    //   - n: Name of the requested file
    RequestMessage(std::string n);

    // Method: serialize
    // Purpose: Serializes the RequestMessage into a byte buffer.
    // Returns:
    //   - A byte buffer representing the serialized RequestMessage.
    Bytes serialize() const;

    // Static Method: deserialize
    // Purpose: Deserializes a byte buffer into a RequestMessage object.
    // Parameters:
    //   - bytes: The byte buffer to deserialize.
    // Returns:
    //   - A RequestMessage object.
    static RequestMessage deserialize(const Bytes& bytes);
};

// Class: StatusMessage
// Purpose: Represents a status message, containing a status flag and an optional message.
//          Provides serialization and deserialization methods.
class StatusMessage {
public:
    bool ok;            // Status flag (true for success, false for failure)
    std::string message; // Optional status message

    // Constructor
    // Parameters:
    //   - ok_: Status flag
    //   - msg: Optional status message
    StatusMessage(bool ok_, std::string msg);

    // Method: serialize
    // Purpose: Serializes the StatusMessage into a byte buffer.
    // Returns:
    //   - A byte buffer representing the serialized StatusMessage.
    Bytes serialize() const;

    // Static Method: deserialize
    // Purpose: Deserializes a byte buffer into a StatusMessage object.
    // Parameters:
    //   - bytes: The byte buffer to deserialize.
    // Returns:
    //   - A StatusMessage object.
    static StatusMessage deserialize(const Bytes& bytes);
};

#endif // PROTOCOL_HPP