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

  // Open a file to read.
  // Returns 0 if operation is successful.
  virtual int Open(const char* path) = 0;

  // Close the reader.
  // Retruns 0 if operation is successful.
  virtual int Close() = 0;

  // Read a record from opened reader.
  // If operation is successful, 1 is returned.
  // If EOF is reached, 0 is returned.
  // Otherwise, negative value indicates an error.
  virtual int Read(UrlFprint* fprint, VisitingRecord* record) = 0;
};

class RecordFileWriter {
public:
  virtual ~RecordFileWriter() {}

  // Open a file to write.
  // Returns 0 if operation is successful.
  virtual int Open(const char* path) = 0;

  // Close the writer.
  // Retruns 0 if operation is successful.
  virtual int Close() = 0;

  // Write a record to opened writer.
  // If operation is successful, 1 is returned.
  // Otherwise, negative value indicates an error.
  virtual int Write(const UrlFprint& fprint, const VisitingRecord& record) = 0;
};

class RecordFileIOFactory {
public:
  // Create a writer.
  // Caller should take care of the returned pointer.
  static RecordFileWriter* CreateWriter();

  // Create a reader.
  // Caller should take care of the returned pointer.
  static RecordFileReader* CreateReader();

private:
  RecordFileIOFactory() {}
};

#endif // SITEMAPSERVICE_RECORDFILEIO_H__

