#include "protocol.hpp"
#include "pack109.hpp"
#include <algorithm>
#include <stdexcept>
#include <iostream>  
#include <iomanip>   


// --- Constructors ---
FileMessage::FileMessage(std::string n, Bytes d)
 : name(std::move(n)), data(std::move(d)) {}

RequestMessage::RequestMessage(std::string n)
 : name(std::move(n)) {}

StatusMessage::StatusMessage(bool ok_, std::string msg)
 : ok(ok_), message(std::move(msg)) {}

// --- XOR‐42 helper ---
Bytes xor42(const Bytes &buf) {
    Bytes out(buf.size());
    for (size_t i = 0; i < buf.size(); ++i)
        out[i] = buf[i] ^ 42;
    return out;
}

// --- FileMessage ---
Bytes FileMessage::serialize() const {
    KVMap inner;
    inner["name"]  = pack109::serialize(name);
    inner["bytes"] = pack109::serialize(data);

    Bytes inner_ser = pack109::serialize_map(inner);
    KVMap outer;
    outer["File"] = inner_ser;

    Bytes full = pack109::serialize_map(outer);
    return xor42(full);
}

FileMessage FileMessage::deserialize(const Bytes &buf) {
    std::cout << "Encrypted response bytes: ";
    for (uint8_t b : buf) std::cout << std::hex << (int)b << " ";
    std::cout << std::endl;

    Bytes decrypted = xor42(buf);
    std::cout << "Decrypted response bytes: ";
    for (uint8_t b : decrypted) std::cout << std::hex << (int)b << " ";
    std::cout << std::endl;

    auto outer = pack109::deserialize_map(decrypted);

    auto it = outer.find("File");
    if (it == outer.end()) throw std::runtime_error("Missing File key");
    Bytes inner_ser = it->second;

    auto inner = pack109::deserialize_map(inner_ser);

    auto itn = inner.find("name");
    if (itn == inner.end()) throw std::runtime_error("Missing name key");
    std::string fname = pack109::deserialize_string(itn->second);

    auto itd = inner.find("bytes");
    if (itd == inner.end()) throw std::runtime_error("Missing bytes key");
    Bytes fdata = pack109::deserialize_vec_u8(itd->second);

    return FileMessage(fname, fdata);
}

// --- RequestMessage ---
Bytes RequestMessage::serialize() const {
    KVMap inner;
    inner["name"] = pack109::serialize(name);

    Bytes inner_ser = pack109::serialize_map(inner);
    KVMap outer;
    outer["Request"] = inner_ser;
    return xor42(pack109::serialize_map(outer));
}

RequestMessage RequestMessage::deserialize(const Bytes &buf) {
    Bytes decrypted = xor42(buf);
    auto outer = pack109::deserialize_map(decrypted);

    auto it = outer.find("Request");
    if (it == outer.end()) throw std::runtime_error("Missing Request key");
    Bytes inner_ser = it->second;

    auto inner = pack109::deserialize_map(inner_ser);
    auto itn = inner.find("name");
    if (itn == inner.end()) throw std::runtime_error("Missing name key");

    std::string fname = pack109::deserialize_string(itn->second);
    return RequestMessage(fname);
}

// --- StatusMessage ---
Bytes StatusMessage::serialize() const {
    KVMap inner;
    inner["ok"]      = pack109::serialize(ok);
    inner["message"] = pack109::serialize(message);

    Bytes inner_ser = pack109::serialize_map(inner);
    KVMap outer;
    outer["Status"] = inner_ser;

    return xor42(pack109::serialize_map(outer));
}

StatusMessage StatusMessage::deserialize(const Bytes &buf) {
    Bytes decrypted = xor42(buf);
    auto outer = pack109::deserialize_map(decrypted);

    auto it = outer.find("Status");
    if (it == outer.end()) throw std::runtime_error("Missing Status key");
    Bytes inner_ser = it->second;

    auto inner = pack109::deserialize_map(inner_ser);

    auto iko = inner.find("ok");
    if (iko == inner.end()) throw std::runtime_error("Missing ok key");
    bool ok_flag = pack109::deserialize_bool(iko->second);

    auto imsg = inner.find("message");
    std::string msg;
    if (imsg != inner.end())
        msg = pack109::deserialize_string(imsg->second);

    return StatusMessage(ok_flag, msg);
}
