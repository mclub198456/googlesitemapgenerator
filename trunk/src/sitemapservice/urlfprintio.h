// Copyright 2008 Google Inc.
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


// This file defines reader/writer for UrlFprint. It reads and writes UrlFprint
// in binary form.

#ifndef SITEMAPSERVICE_URLFPRINTIO_H__
#define SITEMAPSERVICE_URLFPRINTIO_H__

#include <stdio.h>

#include "common/basictypes.h"
#include "common/url.h"

class UrlFprintReader {
 public:
  UrlFprintReader();
  ~UrlFprintReader();

  // Open a file to read.
  // Returns 0 if operation is successful.
  bool Open(const char* path);
  
  // Close the reader.
  // Retruns 0 if operation is successful.
  void Close();

  // Read a record from opened reader.
  // If operation is successful, 1 is returned.
  // If EOF is reached, 0 is returned.
  // Otherwise, negative value indicates an error.
  bool Read(UrlFprint* fprint);

 private:
  FILE* file_;

  DISALLOW_EVIL_CONSTRUCTORS(UrlFprintReader);
};

class UrlFprintWriter {
 public:
  UrlFprintWriter();
  ~UrlFprintWriter();

  // Open a file to write.
  // Returns 0 if operation is successful.
  bool Open(const char* path);
  
  // Close the writer.
  // Retruns 0 if operation is successful.
  void Close();

  // Write a record to opened writer.
  // If operation is successful, 1 is returned.
  // Otherwise, negative value indicates an error.
  bool Write(const UrlFprint& fprint);

 private:
  FILE* file_;

  DISALLOW_EVIL_CONSTRUCTORS(UrlFprintWriter);
};

#endif // SITEMAPSERVICE_URLFPRINTIO_H__

