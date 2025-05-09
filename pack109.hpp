#ifndef PACK109_HPP
#define PACK109_HPP

#include <vector>
#include <string>
#include <map>

// Basic type aliases
using u8 = unsigned char;
using u32 = unsigned int;
using u64 = unsigned long;
using i8 = signed char;
using i32 = signed int;
using i64 = signed long;
using f32 = float;
using f64 = double;
using vec = std::vector<u8>;
using string = std::string;

// Map from string to byte buffers for Pack109
using KVMap = std::map<std::string, vec>;

// Tags for Pack109 types
#define PACK109_TRUE  0xa0
#define PACK109_FALSE 0xa1
#define PACK109_U8    0xa2
#define PACK109_U32   0xa3
#define PACK109_U64   0xa4
#define PACK109_I8    0xa5
#define PACK109_I32   0xa6
#define PACK109_I64   0xa7
#define PACK109_F32   0xa8
#define PACK109_F64   0xa9
#define PACK109_S8    0xaa
#define PACK109_S16   0xab
#define PACK109_A8    0xac
#define PACK109_A16   0xad
#define PACK109_M8    0xae
#define PACK109_M16   0xaf

struct Person {
  u8 age;
  f32 height;
  string name;
};

namespace pack109 {

  // Utility
  void printVec(vec &bytes);

  // Boolean
  vec serialize(bool item);
  bool deserialize_bool(const vec &bytes);

  // Unsigned Integers
  vec serialize(u8 item);
  u8 deserialize_u8(const vec &bytes);
  vec serialize(u32 item);
  u32 deserialize_u32(const vec &bytes);
  vec serialize(u64 item);
  u64 deserialize_u64(const vec &bytes);

  // Signed Integers
  vec serialize(i8 item);
  i8 deserialize_i8(const vec &bytes);
  vec serialize(i32 item);
  i32 deserialize_i32(const vec &bytes);
  vec serialize(i64 item);
  i64 deserialize_i64(const vec &bytes);

  // Floats
  vec serialize(f32 item);
  f32 deserialize_f32(const vec &bytes);
  vec serialize(f64 item);
  f64 deserialize_f64(const vec &bytes);

  // Strings
  vec serialize(const string &item);
  string deserialize_string(const vec &bytes);

  // Arrays
  vec serialize(const std::vector<u8> &item);
  vec serialize(const std::vector<u64> &item);
  vec serialize(const std::vector<f64> &item);
  vec serialize(const std::vector<string> &item);
  std::vector<u8>   deserialize_vec_u8(const vec &bytes);
  std::vector<u64>  deserialize_vec_u64(const vec &bytes);
  std::vector<f64>  deserialize_vec_f64(const vec &bytes);
  std::vector<string> deserialize_vec_string(const vec &bytes);

  // Map serialize/deserialize for byte buffers
  vec serialize_map(const KVMap &m);
  KVMap deserialize_map(const vec &bytes);

  // Structs
  vec serialize(const Person &item);
  Person deserialize_person(const vec &bytes);
}

#endif // PACK109_HPP
