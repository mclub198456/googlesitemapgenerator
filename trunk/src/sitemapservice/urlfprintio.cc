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


#include "sitemapservice/urlfprintio.h"

#include <errno.h>
#include "common/logger.h"

///////////////////////////////////////////////////////////////////////////////
// Implementation of UrlFprintReader

UrlFprintReader::UrlFprintReader() {
  file_ = NULL;
}

UrlFprintReader::~UrlFprintReader() {
  Close();
}

bool UrlFprintReader::Open(const char* path) {
  // File is already opened.
  if (file_ != NULL) {
    return false;
  }

  file_ = fopen(path, "rb");
  return file_ != NULL;
}

void UrlFprintReader::Close() {
  if (file_ == NULL) {
    return;
  }

  fclose(file_);
  file_ = NULL;
}

bool UrlFprintReader::Read(UrlFprint *fprint) {
  if (file_ == NULL) {
    return false;
  }

  size_t result = fread(fprint, sizeof(UrlFprint), 1, file_);

  return result == 1;
}


///////////////////////////////////////////////////////////////////////////////
// Implementation of RecordFileWriter

UrlFprintWriter::UrlFprintWriter() {
  file_ = NULL;
}

UrlFprintWriter::~UrlFprintWriter() {
  Close();
}

bool UrlFprintWriter::Open(const char* path) {
  // File is already opened.
  if (file_ != NULL) {
    Logger::Log(EVENT_ERROR, "Failed to open [%s] to write. (%d)",
              path, errno);
    return false;
  }

  file_ = fopen(path, "wb");
  return file_ != NULL;
}

void UrlFprintWriter::Close() {
  if (file_ == NULL) {
    return;
  }

  fclose(file_);
  file_ = NULL;
}

bool UrlFprintWriter::Write(const UrlFprint& fprint) {
  if (file_ == NULL) {
    return false;
  }

  size_t result = fwrite(&fprint, sizeof(UrlFprint), 1, file_);
  return result == 1;
}

