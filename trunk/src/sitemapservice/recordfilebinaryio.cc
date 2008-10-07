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
  if (file_ != NULL) {
    fclose(file_);
  }
}

bool RecordFileBinaryReader::Initialize(FILE* file) {
  file_ = file;
  return true;
}

int RecordFileBinaryReader::Read(VisitingRecord *record) {
  if (record->url() != NULL) {
    delete record->url();
    record->set_url(NULL);
  }

  size_t result = fread(record, sizeof(VisitingRecord), 1, file_);
  if (result == 1) {
    char* url = new char[record->url_length() + 1];
    result = fread(url, sizeof(char), record->url_length() + 1, file_);
    if (result == record->url_length() + 1) {
      record->set_url(url);
      return 0;
    } else {
      delete[] url;
      record->set_url(NULL);
      return -1;
    }
  } else {
    record->set_url(NULL);
    return 1;
  }
}

void RecordFileBinaryReader::Close() {
  if (file_ != NULL) {
    fclose(file_);
    file_ = NULL;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Implementation of RecordFileWriter

RecordFileBinaryWriter::RecordFileBinaryWriter() {
  file_ = NULL;
}

RecordFileBinaryWriter::~RecordFileBinaryWriter() {
  if (file_ != NULL) {
    fclose(file_);
  }
}

bool RecordFileBinaryWriter::Initialize(FILE* file) {
  file_ = file;
  return true;
}

int RecordFileBinaryWriter::Write(const VisitingRecord& record) {
  if (record.url() == NULL) return 0;

  size_t result = fwrite(&record, sizeof(VisitingRecord), 1, file_);
  if (result == 1) {
    result = fwrite(record.url(), sizeof(char), record.url_length() + 1, file_);
    if (result == record.url_length() + 1) {
      return 0;
    } else {
      return -1;
    }
  } else {
    return 1;
  }
}

void RecordFileBinaryWriter::Close() {
  if (file_ != NULL) {
    fclose(file_);
    file_ = NULL;
  }
}

