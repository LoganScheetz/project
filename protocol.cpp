// File: protocol.cpp
// Description: Implementation of the protocol classes and methods for message serialization,
//              deserialization, and encryption/decryption. Includes FileMessage, RequestMessage,
//              StatusMessage, and XOR-42 encryption utility.
// Author: Logan Scheetz
// Date: 5/12/25

#include "protocol.hpp"
#include "pack109.hpp"
#include <stdexcept>

// --- Constructors ---
// FileMessage constructor
// Parameters:
//   - n: The name of the file.
//   - d: The data of the file as a byte vector.
FileMessage::FileMessage(std::string n, Bytes d)
  : name(std::move(n)), data(std::move(d)) {}

// RequestMessage constructor
// Parameters:
//   - n: The name of the requested file.
RequestMessage::RequestMessage(std::string n)
  : name(std::move(n)) {}

// StatusMessage constructor
// Parameters:
//   - ok_: The status flag (true for success, false for failure).
//   - msg: The status message as a string.
StatusMessage::StatusMessage(bool ok_, std::string msg)
  : ok(ok_), message(std::move(msg)) {}

// --- XOR-42 helper ---
// Function: xor42
// Purpose: Encrypts or decrypts a byte buffer using XOR with a key (default: 42).
// Parameters:
//   - buf: The input byte buffer to be encrypted or decrypted.
//   - key: The key to use for the XOR operation (default is 42).
// Returns:
//   - A new byte buffer after applying the XOR operation.
Bytes xor42(const Bytes &buf, uint8_t key) {
    Bytes out(buf.size());
    for (size_t i = 0; i < buf.size(); ++i)
        out[i] = buf[i] ^ key; // XOR each byte with the key
    return out;
}

// --- FileMessage ---
// Method: serialize
// Purpose: Serializes the FileMessage into a byte buffer.
// Returns:
//   - A byte buffer representing the serialized FileMessage.
Bytes FileMessage::serialize() const {
    KVMap inner;
    inner["name"]  = pack109::serialize(name);  // Serialize the file name
    inner["bytes"] = pack109::serialize(data);  // Serialize the file data

    Bytes inner_ser = pack109::serialize_map(inner); // Serialize the inner KVMap
    KVMap outer{{"File", inner_ser}};                // Wrap in an outer KVMap

    return xor42(pack109::serialize_map(outer));     // Encrypt and return
}

// Method: deserialize
// Purpose: Deserializes a byte buffer into a FileMessage object.
// Parameters:
//   - buf: The byte buffer to deserialize.
// Returns:
//   - A FileMessage object.
// Throws:
//   - runtime_error if the buffer is invalid or missing required keys.
FileMessage FileMessage::deserialize(const Bytes &buf) {
    Bytes decrypted = xor42(buf); // Decrypt the buffer
    auto outer = pack109::deserialize_map(decrypted); // Deserialize the outer map

    auto it = outer.find("File");
    if (it == outer.end()) throw std::runtime_error("Missing File key");
    auto inner = pack109::deserialize_map(it->second); // Deserialize the inner map

    auto itn = inner.find("name");
    if (itn == inner.end()) throw std::runtime_error("Missing name key");
    std::string fname = pack109::deserialize_string(itn->second); // Deserialize file name

    auto itd = inner.find("bytes");
    if (itd == inner.end()) throw std::runtime_error("Missing bytes key");
    Bytes fdata = pack109::deserialize_vec_u8(itd->second); // Deserialize file data

    return FileMessage(fname, fdata);
}

// --- RequestMessage ---
// Method: serialize
// Purpose: Serializes the RequestMessage into a byte buffer.
// Returns:
//   - A byte buffer representing the serialized RequestMessage.
Bytes RequestMessage::serialize() const {
    KVMap inner{{"name", pack109::serialize(name)}};  // Serialize the request name
    KVMap outer{{"Request", pack109::serialize_map(inner)}}; // Wrap in an outer KVMap
    return xor42(pack109::serialize_map(outer)); // Encrypt and return
}

// Method: deserialize
// Purpose: Deserializes a byte buffer into a RequestMessage object.
// Parameters:
//   - buf: The byte buffer to deserialize.
// Returns:
//   - A RequestMessage object.
// Throws:
//   - runtime_error if the buffer is invalid or missing required keys.
RequestMessage RequestMessage::deserialize(const Bytes &buf) {
    Bytes decrypted = xor42(buf); // Decrypt the buffer
    auto outer = pack109::deserialize_map(decrypted); // Deserialize the outer map

    auto it = outer.find("Request");
    if (it == outer.end()) throw std::runtime_error("Missing Request key");
    auto inner = pack109::deserialize_map(it->second); // Deserialize the inner map

    auto itn = inner.find("name");
    if (itn == inner.end()) throw std::runtime_error("Missing name key");
    return RequestMessage(pack109::deserialize_string(itn->second)); // Deserialize and return
}

// --- StatusMessage ---
// Method: serialize
// Purpose: Serializes the StatusMessage into a byte buffer.
// Returns:
//   - A byte buffer representing the serialized StatusMessage.
Bytes StatusMessage::serialize() const {
    KVMap inner;
    inner["ok"]      = pack109::serialize(ok);      // Serialize the status flag
    inner["message"] = pack109::serialize(message); // Serialize the status message
    KVMap outer{{"Status", pack109::serialize_map(inner)}}; // Wrap in an outer KVMap
    return xor42(pack109::serialize_map(outer)); // Encrypt and return
}

// Method: deserialize
// Purpose: Deserializes a byte buffer into a StatusMessage object.
// Parameters:
//   - buf: The byte buffer to deserialize.
// Returns:
//   - A StatusMessage object.
// Throws:
//   - runtime_error if the buffer is invalid or missing required keys.
StatusMessage StatusMessage::deserialize(const Bytes &buf) {
    Bytes decrypted = xor42(buf); // Decrypt the buffer
    auto outer = pack109::deserialize_map(decrypted); // Deserialize the outer map

    auto it = outer.find("Status");
    if (it == outer.end()) throw std::runtime_error("Missing Status key");
    auto inner = pack109::deserialize_map(it->second); // Deserialize the inner map

    auto iko = inner.find("ok");
    if (iko == inner.end()) throw std::runtime_error("Missing ok key");
    bool ok_flag = pack109::deserialize_bool(iko->second); // Deserialize the status flag

    auto imsg = inner.find("message");
    std::string msg;
    if (imsg != inner.end())
        msg = pack109::deserialize_string(imsg->second); // Deserialize the status message if present

    return StatusMessage(ok_flag, msg);
}