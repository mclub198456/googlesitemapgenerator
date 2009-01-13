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


// This file defines the interfaces for VistingRecord reader and writer.
// Both the reader and writer are pure abstract class.
// Besides the interfaces, this file also contains a factory class, which is
// used to create reader/writer instance.

#ifndef SITEMAPSERVICE_RECORDFILEIO_H__
#define SITEMAPSERVICE_RECORDFILEIO_H__

#include "common/url.h"
#include "sitemapservice/visitingrecord.h"


class RecordFileReader {
public:
  virtual ~RecordFileReader() {}

  virtual bool Initialize(FILE* file) = 0;

  // Read a record from opened reader.
  // If operation is successful, 0 is returned.
  // Otherwise, the value indicates an error.
  virtual int Read(VisitingRecord* record) = 0;

  virtual void Close() = 0;
};

class RecordFileWriter {
public:
  virtual ~RecordFileWriter() {}

  virtual bool Initialize(FILE* file) = 0;

  // Write a record to opened writer.
  // If operation is successful, 0 is returned.
  // Otherwise, negative value indicates an error.
  virtual int Write(const VisitingRecord& record) = 0;

  virtual void Close() = 0;
};

class RecordFileIOFactory {
public:
  // Create a writer.
  // Caller should take care of the returned pointer.
  static RecordFileWriter* CreateWriter(const std::string& path);

  // Create a reader.
  // Caller should take care of the returned pointer.
  static RecordFileReader* CreateReader(const std::string& path);

private:
  static const uint64 kVersionA = 200801012108ULL;

  RecordFileIOFactory() {}
};

#endif // SITEMAPSERVICE_RECORDFILEIO_H__

