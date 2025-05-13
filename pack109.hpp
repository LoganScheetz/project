// File: pack109.hpp
// Description: Header file for Pack109 serialization and deserialization utilities.
//              Provides a robust framework for serializing and deserializing primitive
//              types, strings, arrays, maps, and custom structs into byte buffers.
// Author: [Your Name]
// Date: [Insert Date]

#ifndef PACK109_HPP
#define PACK109_HPP

#include <vector>
#include <string>
#include <map>

// Basic type aliases for simplicity and readability
using u8 = unsigned char;     // 8-bit unsigned integer
using u32 = unsigned int;     // 32-bit unsigned integer
using u64 = unsigned long;    // 64-bit unsigned integer
using i8 = signed char;       // 8-bit signed integer
using i32 = signed int;       // 32-bit signed integer
using i64 = signed long;      // 64-bit signed integer
using f32 = float;            // 32-bit floating point number
using f64 = double;           // 64-bit floating point number
using vec = std::vector<u8>;  // Vector of unsigned bytes (byte buffer)
using string = std::string;   // Alias for std::string

// Map type for storing key-value pairs where keys are strings and values are byte buffers
using KVMap = std::map<std::string, vec>;

// Tags for Pack109 types
// These are constants used to identify types during serialization and deserialization
#define PACK109_TRUE  0xa0 // Boolean true
#define PACK109_FALSE 0xa1 // Boolean false
#define PACK109_U8    0xa2 // 8-bit unsigned integer
#define PACK109_U32   0xa3 // 32-bit unsigned integer
#define PACK109_U64   0xa4 // 64-bit unsigned integer
#define PACK109_I8    0xa5 // 8-bit signed integer
#define PACK109_I32   0xa6 // 32-bit signed integer
#define PACK109_I64   0xa7 // 64-bit signed integer
#define PACK109_F32   0xa8 // 32-bit floating point number
#define PACK109_F64   0xa9 // 64-bit floating point number
#define PACK109_S8    0xaa // String with 8-bit size
#define PACK109_S16   0xab // String with 16-bit size
#define PACK109_A8    0xac // Array with 8-bit size
#define PACK109_A16   0xad // Array with 16-bit size
#define PACK109_M8    0xae // Map with 8-bit size
#define PACK109_M16   0xaf // Map with 16-bit size

// Struct: Person
// Represents a simple structure with an age, height, and name.
// Used as an example for struct serialization and deserialization.
struct Person {
    u8 age;         // Age of the person (unsigned 8-bit integer)
    f32 height;     // Height of the person (32-bit floating point number)
    string name;    // Name of the person (string)
};

namespace pack109 {

  // Utility function
  // Prints the contents of a byte vector for debugging purposes
  void printVec(vec &bytes);

  // Serialization and Deserialization for Boolean
  vec serialize(bool item);                  // Serialize a bool into a byte vector
  bool deserialize_bool(const vec &bytes);   // Deserialize a byte vector into a bool

  // Serialization and Deserialization for Unsigned Integers
  vec serialize(u8 item);                    // Serialize an 8-bit unsigned integer
  u8 deserialize_u8(const vec &bytes);       // Deserialize an 8-bit unsigned integer
  vec serialize(u32 item);                   // Serialize a 32-bit unsigned integer
  u32 deserialize_u32(const vec &bytes);     // Deserialize a 32-bit unsigned integer
  vec serialize(u64 item);                   // Serialize a 64-bit unsigned integer
  u64 deserialize_u64(const vec &bytes);     // Deserialize a 64-bit unsigned integer

  // Serialization and Deserialization for Signed Integers
  vec serialize(i8 item);                    // Serialize an 8-bit signed integer
  i8 deserialize_i8(const vec &bytes);       // Deserialize an 8-bit signed integer
  vec serialize(i32 item);                   // Serialize a 32-bit signed integer
  i32 deserialize_i32(const vec &bytes);     // Deserialize a 32-bit signed integer
  vec serialize(i64 item);                   // Serialize a 64-bit signed integer
  i64 deserialize_i64(const vec &bytes);     // Deserialize a 64-bit signed integer

  // Serialization and Deserialization for Floating Points
  vec serialize(f32 item);                   // Serialize a 32-bit float
  f32 deserialize_f32(const vec &bytes);     // Deserialize a 32-bit float
  vec serialize(f64 item);                   // Serialize a 64-bit float
  f64 deserialize_f64(const vec &bytes);     // Deserialize a 64-bit float

  // Serialization and Deserialization for Strings
  vec serialize(const string &item);         // Serialize a string
  string deserialize_string(const vec &bytes); // Deserialize a string

  // Serialization and Deserialization for Arrays
  vec serialize(const std::vector<u8> &item);      // Serialize a vector of unsigned bytes
  vec serialize(const std::vector<u64> &item);     // Serialize a vector of 64-bit unsigned integers
  vec serialize(const std::vector<f64> &item);     // Serialize a vector of 64-bit floats
  vec serialize(const std::vector<string> &item);  // Serialize a vector of strings
  std::vector<u8>   deserialize_vec_u8(const vec &bytes);  // Deserialize into a vector of unsigned bytes
  std::vector<u64>  deserialize_vec_u64(const vec &bytes); // Deserialize into a vector of 64-bit unsigned integers
  std::vector<f64>  deserialize_vec_f64(const vec &bytes); // Deserialize into a vector of 64-bit floats
  std::vector<string> deserialize_vec_string(const vec &bytes); // Deserialize into a vector of strings

  // Serialization and Deserialization for Maps
  vec serialize_map(const KVMap &m);         // Serialize a key-value map
  KVMap deserialize_map(const vec &bytes);   // Deserialize a byte vector into a key-value map

  // Serialization and Deserialization for Structs
  vec serialize(const Person &item);         // Serialize a Person struct
  Person deserialize_person(const vec &bytes); // Deserialize a byte vector into a Person struct
}

#endif // PACK109_HPP