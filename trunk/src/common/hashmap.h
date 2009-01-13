// Copyright 2009 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


// Portable hashmap definition.
// hash_map is not in current C++ standard library. But both Microsoft VC++ and
// GCC contains the implementation. Unfortunately, the two implementations are
// in different namespace, and have slight internal differences.

#ifndef COMMON_HASHMAP_H__
#define COMMON_HASHMAP_H__

#if defined(__linux__) || defined(__unix__)
#include <ext/hash_map>
#include <functional>

#elif defined(WIN32)
// Only compilable with vc++ 8.0, 9.0
// For vc++ 7.1 and older, std::hash_map should be used.
#include <hash_map>

#endif

#include "common/basictypes.h"


#ifdef WIN32

template <class _Kty, class _Ty> struct HashMap {
  typedef stdext::hash_map<_Kty, _Ty> Type;
};


#elif defined(__linux__) || defined(__unix__)

// Compilable with GCC >= 3.x

#if !defined(_STLP_LON_LONG) && !defined(UTIL_HASH_HASH_H__)

namespace __gnu_cxx {

// explicit sepcialization for hash<unsigned long long>
template<>
struct hash<uint64>
: public std::unary_function<uint64, std::size_t> {
  std::size_t operator()(uint64 val) const {
    return static_cast<std::size_t>((val >> 32) ^ val);
  }
};

// explicit sepcialization for hash<string>
template<>
struct hash<std::string>
: public std::unary_function<std::string, std::size_t> {
  std::size_t operator() (const std::string& s) const {
    hash<const char*> h;
    return h(s.c_str());
  }
};

}  // namespace __gnu_cxx
#endif  // !defined(_STLP_LONG_LONG)

template <typename _Kty, typename _Ty> struct HashMap {
  typedef __gnu_cxx::hash_map<_Kty, _Ty> Type;
};

#endif

#endif  // COMMON_HASHMAP_H__
