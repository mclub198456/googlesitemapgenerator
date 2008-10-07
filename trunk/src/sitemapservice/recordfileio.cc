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

#include "sitemapservice/recordfileio.h"

#include "common/logger.h"
#include "sitemapservice/recordfilebinaryio.h"


RecordFileReader* RecordFileIOFactory::CreateReader(const std::string& path) {
  FILE* file = fopen(path.c_str(), "rb");
  if (file == NULL) {
    Logger::Log(EVENT_ERROR, "Failed to open [%s] to read.", path.c_str());
    return NULL;
  }

  uint64 version;
  if (fread(&version, sizeof(uint64), 1, file) != 1) {
    Logger::Log(EVENT_ERROR, "Failed to read version info from [%s].",
              path.c_str());
    fclose(file);
    return NULL;
  }

  RecordFileReader* reader = NULL;
  if (version == kVersionA) {
    reader = new RecordFileBinaryReader();
  } else {
    Logger::Log(EVENT_ERROR, "Unrecognized record file version [%llu] from [%s]",
              version, path.c_str());
  }

  if (reader != NULL) {
    reader->Initialize(file);
  } else {
    fclose(file);
  }

  return reader;
}

RecordFileWriter* RecordFileIOFactory::CreateWriter(const std::string& path) {
  FILE* file = fopen(path.c_str(), "wb");
  if (file == NULL) {
    Logger::Log(EVENT_ERROR, "Failed to open [%s] to write.", path.c_str());
    return NULL;
  }

  uint64 version = kVersionA;
  if (fwrite(&version, sizeof(uint64), 1, file) != 1) {
    Logger::Log(EVENT_ERROR, "Failed to write version info to [%s].",
              path.c_str());
    fclose(file);
    return NULL;
  }

  RecordFileWriter* writer = new RecordFileBinaryWriter();
  writer->Initialize(file);
  return writer;
}

