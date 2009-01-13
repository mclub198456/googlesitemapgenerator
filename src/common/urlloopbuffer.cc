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


#include "common/urlloopbuffer.h"

UrlLoopBuffer::UrlLoopBuffer() {
}

UrlLoopBuffer::~UrlLoopBuffer() {
  // do nothing
}

void UrlLoopBuffer::Initialize() {
  data_->begin = 0;
  data_->end = 0;
}

int UrlLoopBuffer::GetRecordsCount() {
  if (data_->end >= data_->begin) {
    return data_->end - data_->begin;
  } else {
    return data_->end - data_->begin + kUrlBufferSize;
  }
}

// both begin and end shouldn't be changed in this method.
// And only begin can be read in this method.
UrlRecord* UrlLoopBuffer::GetRecord(int index) {
  // Normalize the index.
  index += data_->begin;
  if (index >= kUrlBufferSize) {
    index -= kUrlBufferSize;
  }

  // Validate the index.
  if (index < 0 || index >= kUrlBufferSize) {
    return NULL;
  }

  return &(data_->records[index]);
}

int UrlLoopBuffer::ConsumeRecords(int count) {
  data_->begin += count;
  if (data_->begin >= kUrlBufferSize) {
    data_->begin -= kUrlBufferSize;
  }
  return count;
}

// begin can only be read in this method.
// We should keep it for GetRecord
int UrlLoopBuffer::WriteRecords(UrlRecord* records, int count) {
  int maxend = data_->begin - 1;
  if (maxend < 0) maxend += kUrlBufferSize;

  int i = 0;
  for (; i < count && data_->end != maxend; ++i) {
    data_->records[data_->end] = records[i];
    ++data_->end;
    if (data_->end >= kUrlBufferSize) {
      data_->end -= kUrlBufferSize;
    }
  }
  return i;
}

