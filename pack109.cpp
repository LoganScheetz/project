#include "pack109.hpp"
#include <vector>
#include <stdio.h>
#include <stdexcept>
#include <map>
#include <string>
//added for helper
#include <cstdint>


// Helper to compute the length of a single Pack109 element at offset
static size_t element_length(const vec &bytes, size_t offset) {
    using namespace pack109;
    if (offset >= bytes.size()) 
        throw std::runtime_error("Offset out of range in element_length");

    uint8_t tag = bytes[offset];
    switch (tag) {
        // Booleans
        case PACK109_TRUE:
        case PACK109_FALSE:
            return 1;               // only the tag byte

        // 1-byte integers
        case PACK109_U8:
        case PACK109_I8:
            return 2;               // tag + 1 data byte

        // 4-byte types (u32, i32, f32)
        case PACK109_U32:
        case PACK109_I32:
        case PACK109_F32:
            return 1 + 4;           // tag + 4 data bytes

        // 8-byte types (u64, i64, f64)
        case PACK109_U64:
        case PACK109_I64:
        case PACK109_F64:
            return 1 + 8;           // tag + 8 data bytes

        // String8: tag, length byte, then that many chars
        case PACK109_S8: {
            if (offset + 1 >= bytes.size())
                throw std::runtime_error("Truncated S8 header");
            uint8_t len = bytes[offset + 1];
            return 2 + len;        // tag + len byte + data
        }

        // Array8 of u8 elements: tag, count, then each element as (U8 tag + byte)
        case PACK109_A8: {
            if (offset + 1 >= bytes.size())
                throw std::runtime_error("Truncated A8 header");
            uint8_t count = bytes[offset + 1];
            size_t pos = offset + 2;
            size_t total = 2;
            for (int i = 0; i < count; ++i) {
                if (pos + 1 >= bytes.size() || bytes[pos] != PACK109_U8)
                    throw std::runtime_error("Malformed A8 element");
                total += 2;         // each U8 element: tag + 1 byte
                pos   += 2;
            }
            return total;
        }

        // Map8: tag, count, then [ key, value ] pairs
        case PACK109_M8: {
            if (offset + 1 >= bytes.size())
                throw std::runtime_error("Truncated M8 header");
            uint8_t count = bytes[offset + 1];
            size_t pos   = offset + 2;
            size_t total = 2;
            for (int i = 0; i < count; ++i) {
                // key (always S8)
                size_t klen = element_length(bytes, pos);
                pos   += klen;
                total += klen;
                // value (any element)
                size_t vlen = element_length(bytes, pos);
                pos   += vlen;
                total += vlen;
            }
            return total;
        }

        default:
            throw std::runtime_error(
                std::string("Unsupported tag in element_length: ") 
                + std::to_string(tag));
    }
}

namespace pack109 {

vec serialize_map(const KVMap &m) {
    if (m.size() > 255) {
        throw std::runtime_error("Map too large");
    }

    vec bytes;
    bytes.push_back(PACK109_M8);            // map tag
    bytes.push_back(static_cast<u8>(m.size()));  // entry count

    for (const auto &p : m) {
        // Key is always a string → use serialize(string)
        vec keyb = serialize(p.first);
        // Value must already be a tagged element (we built it via serialize(...) before)
        vec valb = p.second;

        // Append key then value directly
        bytes.insert(bytes.end(), keyb.begin(), keyb.end());
        bytes.insert(bytes.end(), valb.begin(), valb.end());
    }

    return bytes;
}



KVMap deserialize_map(const vec &bytes) {
    if (bytes.size() < 2 || bytes[0] != PACK109_M8)
        throw std::runtime_error("Invalid map format");
    uint8_t count = bytes[1];
    KVMap out;
    size_t pos = 2;
    for (int i = 0; i < count; ++i) {
        // Deserialize key
        if (pos >= bytes.size() || bytes[pos] != PACK109_S8)
            throw std::runtime_error("Invalid map key format");
        size_t klen = element_length(bytes, pos);
        std::string key(bytes.begin() + pos + 2,
                        bytes.begin() + pos + klen);
        pos += klen;

        // Deserialize value
        size_t vlen = element_length(bytes, pos);
        vec val(bytes.begin() + pos, bytes.begin() + pos + vlen);
        out[key] = val;
        pos += vlen;
    }

    // if (pos != bytes.size())
    //     throw std::runtime_error("Extra data after map deserialization");

    return out;
}

vec serialize(bool item) {
  vec bytes;
  bytes.push_back(item ? PACK109_TRUE : PACK109_FALSE);
  return bytes;
}

bool deserialize_bool(const vec &bytes) {
  if (bytes.size() < 1)
    throw std::runtime_error("Invalid boolean format");
  if (bytes[0] == PACK109_TRUE)
    return true;
  if (bytes[0] == PACK109_FALSE)
    return false;
  throw std::runtime_error("Invalid boolean tag");
}

vec serialize(u8 item) {
  vec bytes;
  bytes.push_back(PACK109_U8);
  bytes.push_back(item);
  return bytes;
}

u8 deserialize_u8(const vec &bytes) {
  if (bytes.size() != 2 || bytes[0] != PACK109_U8)
    throw std::runtime_error("Invalid u8 format");
  return bytes[1];
}

vec serialize(u32 item) {
  vec bytes;
  bytes.push_back(PACK109_U32);
  for (int i = 3; i >= 0; --i) bytes.push_back((item >> (8*i)) & 0xFF);
  return bytes;
}

u32 deserialize_u32(const vec &bytes) {
  if (bytes.size() != 5 || bytes[0] != PACK109_U32)
    throw std::runtime_error("Invalid u32 format");
  u32 v = 0;
  for (int i = 1; i <= 4; ++i) v = (v << 8) | bytes[i];
  return v;
}

vec serialize(u64 item) {
  vec bytes;
  bytes.push_back(PACK109_U64);
  for (int i = 7; i >= 0; --i) bytes.push_back((item >> (8*i)) & 0xFF);
  return bytes;
}

u64 deserialize_u64(const vec &bytes) {
  if (bytes.size() != 9 || bytes[0] != PACK109_U64)
    throw std::runtime_error("Invalid u64 format");
  u64 v = 0;
  for (int i = 1; i <= 8; ++i) v = (v << 8) | bytes[i];
  return v;
}

vec serialize(i8 item) {
  vec bytes;
  bytes.push_back(PACK109_I8);
  bytes.push_back((u8)item);
  return bytes;
}

i8 deserialize_i8(const vec &bytes) {
  if (bytes.size() != 2 || bytes[0] != PACK109_I8)
    throw std::runtime_error("Invalid i8 format");
  return (i8)bytes[1];
}

vec serialize(i32 item) {
  vec bytes;
  bytes.push_back(PACK109_I32);
  for (int i = 3; i >= 0; --i) bytes.push_back((item >> (8*i)) & 0xFF);
  return bytes;
}

i32 deserialize_i32(const vec &bytes) {
  if (bytes.size() != 5 || bytes[0] != PACK109_I32)
    throw std::runtime_error("Invalid i32 format");
  i32 v = 0;
  for (int i = 1; i <= 4; ++i) v = (v << 8) | (uint8_t)bytes[i];
  return v;
}

vec serialize(i64 item) {
  vec bytes;
  bytes.push_back(PACK109_I64);
  for (int i = 7; i >= 0; --i) bytes.push_back((item >> (8*i)) & 0xFF);
  return bytes;
}

i64 deserialize_i64(const vec &bytes) {
  if (bytes.size() != 9 || bytes[0] != PACK109_I64)
    throw std::runtime_error("Invalid i64 format");
  i64 v = 0;
  for (int i = 1; i <= 8; ++i) v = (v << 8) | bytes[i];
  return v;
}

vec serialize(f32 item) {
  vec bytes;
  bytes.push_back(PACK109_F32);
  u32 raw = *reinterpret_cast<u32*>(&item);
  for (int i = 3; i >= 0; --i) bytes.push_back((raw >> (8*i)) & 0xFF);
  return bytes;
}

f32 deserialize_f32(const vec &bytes) {
  if (bytes.size() != 5 || bytes[0] != PACK109_F32)
    throw std::runtime_error("Invalid f32 format");
  u32 raw = 0;
  for (int i = 1; i <= 4; ++i) raw = (raw << 8) | bytes[i];
  return *reinterpret_cast<f32*>(&raw);
}

vec serialize(f64 item) {
  vec bytes;
  bytes.push_back(PACK109_F64);
  u64 raw = *reinterpret_cast<u64*>(&item);
  for (int i = 7; i >= 0; --i) bytes.push_back((raw >> (8*i)) & 0xFF);
  return bytes;
}

f64 deserialize_f64(const vec &bytes) {
  if (bytes.size() != 9 || bytes[0] != PACK109_F64)
    throw std::runtime_error("Invalid f64 format");
  u64 raw = 0;
  for (int i = 1; i <= 8; ++i) raw = (raw << 8) | bytes[i];
  return *reinterpret_cast<f64*>(&raw);
}

vec serialize(const string &item) {
  if (item.size() > 255) throw std::runtime_error("String too long");
  vec bytes;
  bytes.push_back(PACK109_S8);
  bytes.push_back((u8)item.size());
  for (char c: item) bytes.push_back((u8)c);
  return bytes;
}

string deserialize_string(const vec &bytes) {
  if (bytes.size() < 2 || bytes[0] != PACK109_S8) throw std::runtime_error("Invalid string format");
  u8 len = bytes[1];
  if (bytes.size() != (size_t)2+len) throw std::runtime_error("String length mismatch");
  return string(bytes.begin()+2, bytes.end());
}

vec serialize(const std::vector<u8> &items) {
  if (items.size() > 255) throw std::runtime_error("Vector<u8> too long");
  vec bytes;
  bytes.push_back(PACK109_A8);
  bytes.push_back((u8)items.size());
  for (u8 v: items) {
    vec tmp = serialize(v);
    bytes.insert(bytes.end(), tmp.begin(), tmp.end());
  }
  return bytes;
}

std::vector<u8> deserialize_vec_u8(const vec &bytes) {
  if (bytes.size()<2 || bytes[0]!=PACK109_A8) throw std::runtime_error("Invalid vec_u8 format");
  u8 len = bytes[1];
  std::vector<u8> out;
  size_t i=2;
  while (out.size()<len) {
    if (i+2>bytes.size()||bytes[i]!=PACK109_U8) throw std::runtime_error("Malformed u8 in vector");
    out.push_back(bytes[i+1]); i+=2;
  }
  return out;
}

vec serialize(const std::vector<u64> &items) {
  if (items.size()>255) throw std::runtime_error("Vector<u64> too long");
  vec bytes{PACK109_A8,(u8)items.size()};
  for(u64 v:items) {
    vec tmp=serialize(v);
    bytes.insert(bytes.end(),tmp.begin(),tmp.end());
  }
  return bytes;
}

std::vector<u64> deserialize_vec_u64(const vec &bytes) {
  if (bytes.size()<2||bytes[0]!=PACK109_A8) throw std::runtime_error("Invalid vec_u64");
  u8 len=bytes[1]; std::vector<u64> out; size_t off=2;
  for(int i=0;i<len;i++){
    if(off+9>bytes.size()) throw std::runtime_error("Truncated u64");
    vec slice(bytes.begin()+off,bytes.begin()+off+9);
    out.push_back(deserialize_u64(slice)); off+=9;
  }
  return out;
}

vec serialize(const std::vector<f64> &items) {
  if (items.size()>255) throw std::runtime_error("Vector<f64> too long");
  vec bytes{PACK109_A8,(u8)items.size()};
  for(f64 v:items){ vec tmp=serialize(v); bytes.insert(bytes.end(),tmp.begin(),tmp.end()); }
  return bytes;
}

std::vector<f64> deserialize_vec_f64(const vec &bytes) {
  if(bytes.size()<2||bytes[0]!=PACK109_A8) throw std::runtime_error("Invalid vec_f64");
  u8 len=bytes[1]; std::vector<f64> out; size_t off=2;
  for(int i=0;i<len;i++){
    if(off+9>bytes.size()) throw std::runtime_error("Truncated f64");
    vec slice(bytes.begin()+off,bytes.begin()+off+9);
    out.push_back(deserialize_f64(slice)); off+=9;
  }
  return out;
}

vec serialize(const std::vector<string> &items) {
  if(items.size()>255) throw std::runtime_error("Vector<string> too long");
  vec bytes{PACK109_A8,(u8)items.size()};
  for(const auto &s:items){ vec tmp=serialize(s); bytes.insert(bytes.end(),tmp.begin(),tmp.end()); }
  return bytes;
}

std::vector<string> deserialize_vec_string(const vec &bytes) {
  if(bytes.size()<2||bytes[0]!=PACK109_A8) throw std::runtime_error("Invalid vec_string");
  u8 len=bytes[1]; std::vector<string> out; size_t off=2;
  for(int i=0;i<len;i++){
    if(off+2>bytes.size()||bytes[off]!=PACK109_S8) throw std::runtime_error("Malformed string in vec");
    u8 sl=bytes[off+1]; if(off+2+sl>bytes.size()) throw std::runtime_error("Truncated string");
    vec slice(bytes.begin()+off,bytes.begin()+off+2+sl);
    out.push_back(deserialize_string(slice)); off+=2+sl;
  }
  return out;
}




vec serialize(const Person &item) {
  KVMap m;
  m["age"] = serialize(item.age);
  m["height"] = serialize(item.height);
  m["name"] = serialize(item.name);
  return serialize_map(m);
}

Person deserialize_person(const vec &bytes) {
  Person p;
  auto m = deserialize_map(bytes);
  { auto b=m["age"]; p.age = deserialize_u8(b);}  
  { auto b=m["height"]; p.height = deserialize_f32(b);}  
  { auto b=m["name"]; p.name = deserialize_string(b);}  
  return p;
}

void printVec(vec &bytes) {
  printf("[ ");
  for (u8 b : bytes) printf("%02x ", b);
  printf("]\n");
}

} // namespace pack109
