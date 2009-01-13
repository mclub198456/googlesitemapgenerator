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


// This header file defines the basic types, basic constants and macros widely
// used in this application.

#ifndef COMMON_BASICTYPES_H__
#define COMMON_BASICTYPES_H__

typedef unsigned int       uint32;
typedef long long          int64;
typedef unsigned long long uint64;

// Constants to define the max length of common values.
const int kMaxUrlLength = 512;
const int kMaxHostLength = 256;
const int kMaxSiteIdLength = 256;
const int kMaxErrLength = 512;

// A macro to disallow the evil copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_EVIL_CONSTRUCTORS(TypeName)    \
  TypeName(const TypeName&);                    \
  void operator=(const TypeName&)

// A macro to disallow all the implicit constructors, namely the default
// constructor, copy constructor and operator= functions.
//
// This should be used in the private: declarations for a class that wants to
// prevent anyone from instantiating it. This is especially useful for classes
// containing only static methods.
#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
  TypeName();                                    \
  DISALLOW_EVIL_CONSTRUCTORS(TypeName)

#endif  // COMMON_BASICTYPES_H__
