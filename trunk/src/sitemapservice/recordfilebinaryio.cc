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


#include "sitemapservice/recordfilebinaryio.h"

///////////////////////////////////////////////////////////////////////////////
// Implementation of RecordFileBinaryReader

RecordFileBinaryReader::RecordFileBinaryReader() {
  file_ = NULL;
}

RecordFileBinaryReader::~RecordFileBinaryReader() {
  Close();
}

int RecordFileBinaryReader::Open(const char* path) {
  // File is already opened.
  if (file_ != NULL) {
    return 1;
  }

  file_ = fopen(path, "rb");
  return file_ != NULL ? 0 : 1;
}

int RecordFileBinaryReader::Close() {
  if (file_ == NULL) {
    return 0;
  }

  int result = fclose(file_);
  if (result == 0) {
    file_ = NULL;
  }
  return result;
}

int RecordFileBinaryReader::Read(UrlFprint *fprint, VisitingRecord *record) {
  size_t result = fread(fprint, sizeof(UrlFprint), 1, file_);
  if (result == 1) {
    result = fread(record, sizeof(VisitingRecord), 1, file_);
  }

  return result == 1 ? 0 : 1;
}


///////////////////////////////////////////////////////////////////////////////
// Implementation of RecordFileWriter

RecordFileBinaryWriter::RecordFileBinaryWriter() {
  file_ = NULL;
}

RecordFileBinaryWriter::~RecordFileBinaryWriter() {
  Close();
}

int RecordFileBinaryWriter::Open(const char* path) {
  // File is already opened.
  if (file_ != NULL) {
    return 1;
  }

  file_ = fopen(path, "wb");
  return file_ != NULL ? 0 : 1;
}

int RecordFileBinaryWriter::Close() {
  if (file_ == NULL) {
    return 0;
  }

  int result = fclose(file_);
  if (result == 0) {
    file_ = NULL;
  }
  return result;
}

int RecordFileBinaryWriter::Write(const UrlFprint& fprint, const VisitingRecord& record) {
  size_t result = fwrite(&fprint, sizeof(UrlFprint), 1, file_);
  if (result == 1) {
    result = fwrite(&record, sizeof(VisitingRecord), 1, file_);
  }
  return result == 1 ? 0 : 1;
}

