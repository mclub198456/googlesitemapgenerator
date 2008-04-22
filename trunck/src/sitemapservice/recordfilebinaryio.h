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


// This file implements RecordFileReader and RecordFileWriter interfaces.
// The visiting records are written/read in binary form.
// This is efficient, but may waste some spaces because max length of URL is
// much larger than actual value.

#ifndef SITEMAPSERVICE_RECORDFILEBINARYIO_H__
#define SITEMAPSERVICE_RECORDFILEBINARYIO_H__

#include "common/url.h"
#include "sitemapservice/visitingrecord.h"
#include "sitemapservice/recordfileio.h"

class RecordFileBinaryReader : public RecordFileReader {
public:
  RecordFileBinaryReader();
  virtual ~RecordFileBinaryReader();

  // Overriden methods. See base class.
  virtual int Open(const char* path);
  virtual int Close();
  virtual int Read(UrlFprint* fprint, VisitingRecord* record);

private:
  // Disallow copy constructor.
  RecordFileBinaryReader(const RecordFileBinaryReader&) {}

  FILE* file_;
};

class RecordFileBinaryWriter : public RecordFileWriter {
public:
  RecordFileBinaryWriter();
  virtual ~RecordFileBinaryWriter();

  // Overridden methods. See base class.
  virtual int Open(const char* path);
  virtual int Close();
  virtual int Write(const UrlFprint& fprint, const VisitingRecord& record);

private:
  // Disallow copy constructor.
  RecordFileBinaryWriter(const RecordFileBinaryWriter&) {}

  FILE* file_;
};

#endif // SITEMAPSERVICE_RECORDFILEBINARYIO_H__

