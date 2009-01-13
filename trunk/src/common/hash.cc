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


#include "common/hash.h"

// Generate finger print for a string.
// Both 31 and 33 are fast multipliers, which can be implemented by shift and
// add operator. And they are also widely adopted hash function multiplier.
uint64 FingerPrint(const char *s) {
  uint32 high = 0, low = 0;
  for (; (*s) != '\0'; ++s) {
    low = (low << 5) - low + (*s);
    high = (high << 5) + high + (*s);
  }

  return (static_cast<uint64>(high) << 32) + low;
}
