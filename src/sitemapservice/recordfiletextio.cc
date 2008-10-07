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


#include "sitemapservice/recordfiletextio.h"

///////////////////////////////////////////////////////////////////////////////
// Implementation of RecordFileTextReader

RecordFileTextReader::RecordFileTextReader() {
  file_ = NULL;
}

RecordFileTextReader::~RecordFileTextReader() {
  Close();
}

int RecordFileTextReader::Open(const char* path) {
  // File is already opened.
  if (file_ != NULL) {
    return 1;
  }

  file_ = fopen(path, "r");
  return file_ != NULL ? 0 : 1;
}

int RecordFileTextReader::Close() {
  if (file_ == NULL) {
    return 0;
  }

  int result = fclose(file_);
  if (result == 0) {
    file_ = NULL;
  }
  return result;
}

int RecordFileTextReader::Read(UrlFprint *fprint, VisitingRecord *record) {

  int result = fscanf(file_, "%llu ", fprint);
  if (result == EOF) return -1;

  // WARN !!!
  // The url shouldn't be empty, nor contains whitespace.
  // And of course, it should be end with '\0'.
  // 511 is equal to kMaxUrlLength - 1. 
  // We don't have a good way to use kMaxUrlLength directly here.
  result = fscanf(file_, "%511s", record->url());
  if (result == EOF) return -1;

  // time_t may have different type in other platform according to old C standard
  // It should be defined as int64 or uint64 as which in C99.
#ifdef WIN32 
  result = fscanf(file_, "%lld %lld %lld %d %d %lld",
                  &record->first_appear, &record->last_access,
                  &record->last_change, &record->count_access,
                  &record->count_change, &record->last_content);
#elif defined(__linux__)
  result = fscanf(file_, "%ld %ld %ld %d %d %lld",
                  &record->first_appear, &record->last_access,
                  &record->last_change, &record->count_access,
                  &record->count_change, &record->last_content);
#endif // WIN32
  if (result == EOF) return -1;

  return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Implementation of RecordFileWriter

RecordFileTextWriter::RecordFileTextWriter() {
  file_ = NULL;
}

RecordFileTextWriter::~RecordFileTextWriter() {
  Close();
}

int RecordFileTextWriter::Open(const char* path) {
  // file is already opened
  if (file_ != NULL) {
    return 1;
  }

  file_ = fopen(path, "w");
  return file_ != NULL ? 0 : 1;
}

int RecordFileTextWriter::Close() {
  if (file_ == NULL) {
    return 0;
  }

  int result = fclose(file_);
  if (result == 0) {
    file_ = NULL;
  }
  return result;
}

// Wite the record in human readable format.
int RecordFileTextWriter::Write(const UrlFprint& fprint, const VisitingRecord& record) {

  fprintf(file_, "%llu ", fprint);
  fprintf(file_, "%s\n", record.url()); // here we assume url doesn't contain whitespaces

  // time_t may have different type in other platform according to old C standard
  // It should be defined as int64 or uint64 as which in C99.
#ifdef WIN32 
  fprintf(file_, "%lld %lld %lld %d %d %lld\n",
    record.first_appear, record.last_access, record.last_change,
    record.count_access, record.count_change, record.last_content);
#elif defined(__linux__)
  fprintf(file_, "%ld %ld %ld %d %d %lld\n",
    record.first_appear, record.last_access, record.last_change,
    record.count_access, record.count_change, record.last_content);
#endif // WIN32

  return 0;
}



