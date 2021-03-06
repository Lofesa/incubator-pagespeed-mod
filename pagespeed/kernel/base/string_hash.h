/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */


#ifndef PAGESPEED_KERNEL_BASE_STRING_HASH_H_
#define PAGESPEED_KERNEL_BASE_STRING_HASH_H_

#include <cstddef>

#include "pagespeed/kernel/base/string.h"
#include "pagespeed/kernel/base/string_util.h"

namespace net_instaweb {

// A hash function for strings that can be used both in a case-sensitive
// and case-insensitive way
template<class CharTransform, typename IntType>
inline IntType HashString(const char* s, size_t len) {
  // This implemention is based on code in
  // third_party/chromium/src/base/hash_tables.h.
  IntType result = 0;
  for (const char* end = s + len; s != end; ++s) {
    result = (result * 131) + CharTransform::Normalize(*s);
  }
  return result;
}

// Combine two hash values in a reasonable way.  Here to avoid
// excessive mysticism in the remainder of the code.
inline size_t JoinHash(size_t a, size_t b) {
  return (a + 56) * 137 + b * 151;  // Uses different prime multipliers.
}

// A helper for case-sensitive hashing
struct CasePreserve {
  // We want to use unsigned characters for the return value of Normalize
  // here and in CaseFold::Normalize.  This is so that we get the same
  // hash-value arithmetic regardless of whether the c++ compiler treats
  // chars as signed or unsigned by default.  We want to get the same
  // hash-values independent of machine so that we get consistent domain
  // sharding and therefore better caching behavior in a multi-server setup
  // that contains heterogeneous machines.
  static unsigned char Normalize(char c) {
    return c;
  }

  static bool Compare(const StringPiece& a, const StringPiece& b) {
    return a < b;
  }
};

// A helper for case-insensitive hashing, which folds to lowercase
struct CaseFold {
  static unsigned char Normalize(char c) {
    return LowerChar(c);
  }

  static bool Compare(const StringPiece& a, const StringPiece& b) {
    return StringCaseCompare(a, b) < 0;
  }
};

// Functors for constructing case-insensitive and case-sensitive hash-tables.
struct CasePreserveStringHash {
  size_t operator()(const GoogleString& str) const {
    return HashString<CasePreserve, size_t>(str.data(), str.size());
  }
};

struct CaseFoldStringHash {
  size_t operator()(const GoogleString& str) const {
    return HashString<CaseFold, size_t>(str.data(), str.size());
  }
};

struct CaseFoldStringEqual {
  bool operator()(const GoogleString& a, const GoogleString& b) const {
    return MemCaseEqual(a.data(), a.size(), b.data(), b.size());
  }
};

struct CasePreserveStringPieceHash {
  size_t operator()(StringPiece str) const {
    return HashString<CasePreserve, size_t>(str.data(), str.size());
  }
};

struct CaseFoldStringPieceHash {
  size_t operator()(StringPiece str) const {
    return HashString<CaseFold, size_t>(str.data(), str.size());
  }
};
struct CaseFoldStringPieceEqual {
  bool operator()(StringPiece a, StringPiece b) const {
    return MemCaseEqual(a.data(), a.size(), b.data(), b.size());
  }
};

}  // namespace net_instaweb

#endif  // PAGESPEED_KERNEL_BASE_STRING_HASH_H_
