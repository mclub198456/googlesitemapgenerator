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


// UrlLoopBuffer is a loop buffer where UrlRecord can be communicated.
// This buffer is designed for one reader and multiple writers. And please note
// that, this UrlLoopBuffer is not process/thread-safe. External mechanism
// should be used to achieve synchronization.
// UrlLoopBuffer doesn't contains buffering memeory actually. It contains a
// pointer to UrlBufferData, which is the actual buffering place. UrlLoopBuffer
// only provides operations on the UrlBufferData.

#ifndef COMMON_URLLOOPBUFFER_H__
#define COMMON_URLLOOPBUFFER_H__

#include "common/urlrecord.h"

const int kUrlBufferSize = 1000;

struct UrlBufferData {
  int begin;
  int end;

  // Available records are in the range of [begin, end)
  // There are at most kUrlBufferSize - 1 records available.
  UrlRecord records[kUrlBufferSize];
};

// This loop buffer is designed for one reader, multiple writers.
// Please note there is no synchronization on this buffer.
class UrlLoopBuffer {
 public:
  UrlLoopBuffer();
  ~UrlLoopBuffer();

  void Initialize();

  // The three steps for reader.
  // 1) Get available records in buffer.
  int GetRecordsCount();
  // 2) Get record at "index" position.
  // This index value should be in the range of [0, GetRecordsCount()).
  UrlRecord* GetRecord(int index);
  // 3) Consume the records, the count should be same with GetRecordsCount().
  int ConsumeRecords(int count);

  // Writer needs only one call.
  int WriteRecords(UrlRecord* records, int count);

  // Get/set the internal buffering data.
  UrlBufferData* GetInternalData() {return data_;}
  void SetInternalData(UrlBufferData* data) {data_ = data;}

 private:
  UrlBufferData* data_;
};

#endif  // COMMON_URLLOOPBUFFER_H__





