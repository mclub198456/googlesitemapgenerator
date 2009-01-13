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

#include "common/urlpipe.h"
#include "common/logger.h"

// Constant names for the events and shared memory
const char* UrlPipe::kSharedMemName = "GOOGLE_SITEMAP_GENERATOR_URLPIPE_SHM";
const char* UrlPipe::kRwMutexName = "GOOGLE_SITEMAP_GENERATOR_URLPIPE_RW";
const char* UrlPipe::kNotifyMutexName = "GOOGLE_SITEMAP_GENERATOR_URLPIPE_NOTIFY";

UrlPipe::UrlPipe(void) {
  records_ = NULL;
}

UrlPipe::~UrlPipe(void) {
  ReleaseResource();

  if (records_ != NULL) {
    delete[] records_;
    records_ = NULL;
  }
}

void UrlPipe::ReleaseResource() {
  shared_mem_.Destroy();
  buffer_.SetInternalData(NULL);

  rw_mutex_.Destroy();
  notify_mutex_.Destroy();
  mutex_set_.Destroy();
}

bool UrlPipe::AcquireResource() {
  time(&last_acquire_resource_);

  if (!mutex_set_.Load()) {
    Logger::Log(EVENT_ERROR, "Failed to load mutex set for url pipe.");
    return false;
  }

  if (!mutex_set_.GetMutex(kRwMutexName, true, false, &rw_mutex_)) {
    Logger::Log(EVENT_ERROR, "Failed to get rw mutex.");
    return false;
  }

  if (!mutex_set_.GetMutex(kNotifyMutexName, false, true, &notify_mutex_)) {
    Logger::Log(EVENT_ERROR, "Failed to get notify mutex.");
    return false;
  }

  bool shared_mem_exist;
  if (!shared_mem_.Create(kSharedMemName, sizeof(UrlBufferData),
                     is_receiver_, &shared_mem_exist)) {
    Logger::Log(EVENT_ERROR, "Failed to create shared memory.");
    return false;
  }

  buffer_.SetInternalData(reinterpret_cast<UrlBufferData*>(shared_mem_.data()));
  if (shared_mem_exist == false) {
    buffer_.Initialize();
  }
  return true;
}

bool UrlPipe::Initialize(bool is_receiver) {
  // Initialize mutex set.
  if (!mutex_set_.Initialize("urlpipe_mutexset", is_receiver)) {
    return false;
  }
  mutex_set_.RegisterMutex(kRwMutexName);
  mutex_set_.RegisterMutex(kNotifyMutexName);

  // Do some role based initialization.
  is_receiver_ = is_receiver;
  if (is_receiver) {
    records_ = new UrlRecord[kUrlBufferSize];
  }

  if (!AcquireResource()) {
    Logger::Log(EVENT_IMPORTANT, "UrlPipe failed to acquire resource.");
    if (is_receiver) {
      // Receiver has no chance to reload resource.
      return false;
    }
  }
  return true;
}

int UrlPipe::Send(UrlRecord *urlrecord, int count) {
  Mutex::MutexResult result;
  do {
    // Try to obtain rw mutex.
    result = rw_mutex_.Wait(kSendWaitTime);
    if (result != Mutex::MUTEX_OK) {
      Logger::Log(EVENT_NORMAL, "Sender can't wait RW mutex. (%d|%d)",
                result, rw_mutex_.LastError());
      break;
    }

    // Write record to buffer.
    int writecount = buffer_.WriteRecords(urlrecord, count);

    // Release RW mutex.
    result = rw_mutex_.Post();
    if (result != Mutex::MUTEX_OK) {
      Logger::Log(EVENT_NORMAL, "Sender can't post RW mutex. (%d|%d)",
                result, rw_mutex_.LastError());
      break;
    }

    // Set NOTIFY event to notify receiver.
    result = notify_mutex_.Post();
    if (result != Mutex::MUTEX_OK) {
      Logger::Log(EVENT_NORMAL, "Sender can't post NOTIFY event. (%d|%d)",
                result, notify_mutex_.LastError());
      break;
    }

    return writecount;
  } while (false);

  if (result == Mutex::MUTEX_TIMEOUT) {
    Logger::Log(EVENT_NORMAL, "Sender's waiting is time-out.");
    return 0;
  }

  // Error occurs.
  ReleaseResource();

  // Try to reload the resource.
  if (result == Mutex::MUTEX_INVALID) {
    time_t now = time(NULL);
    if (last_acquire_resource_ + kRetrievePeriod < now) {
      if (AcquireResource()) {
        return Send(urlrecord, count);
      }
    }
  }

  return -1;
}

int UrlPipe::Receive(UrlRecord** records) {
  Mutex::MutexResult result;

  // Wait for NOTIFY and RW event to get URL count in buffer.
  result = notify_mutex_.Wait(Mutex::kWaitInfinite);
  if (result != Mutex::MUTEX_OK) {
    Logger::Log(EVENT_NORMAL, "Receiver can't wait notify (%d|%d).",
              result, notify_mutex_.LastError());
    return -1;
  }

  result = rw_mutex_.Wait(Mutex::kWaitInfinite);
  if (result != Mutex::MUTEX_OK) {
    Logger::Log(EVENT_NORMAL, "Receiver can't wait rw-1 (%d|%d).",
              result, rw_mutex_.LastError());
    return -1;
  }

  // Read records count in buffer.
  int count = buffer_.GetRecordsCount();

  // Release RW mutex.
  result = rw_mutex_.Post();
  if (result != Mutex::MUTEX_OK) {
    Logger::Log(EVENT_NORMAL, "Receiver can't post rw-1 (%d|%d)",
              result, rw_mutex_.LastError());
    return -1;
  }

  // Read records from buffer.
  for (int i = 0; i < count; ++i) {
    UrlRecord* record = buffer_.GetRecord(i);
    if (record != NULL) {
      records_[i] = *record;
    } else {
      Logger::Log(EVENT_NORMAL, "Receiver encounter invalid record.");
    }
  }
  *records = records_;

  // Move index in buffer to indicate the records have been read.
  result = rw_mutex_.Wait(Mutex::kWaitInfinite);
  if (result != Mutex::MUTEX_OK) {
    Logger::Log(EVENT_ERROR, "Receiver can't wait rw-2 (%d|%d).",
              result, rw_mutex_.LastError());
    return -1;
  }
  buffer_.ConsumeRecords(count);
  result = rw_mutex_.Post();
  if (result != Mutex::MUTEX_OK) {
    Logger::Log(EVENT_NORMAL, "Receiver can't post RW-2 (%d|%d).",
              result, rw_mutex_.LastError());
    return -1;
  }

  return count;
}
