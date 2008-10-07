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


#include "common/messagepipe.h"

#include "common/logger.h"

// Constant names for the events and shared memory
const char* MessagePipe::kSharedMemName = "GOOGLE_SITEMAP_GENERATOR_CGIMSG_SHM";
const char* MessagePipe::kServerMutexName = "GOOGLE_SITEMAP_GENERATOR_CGIMSG_SERVER";
const char* MessagePipe::kClientMutexName = "GOOGLE_SITEMAP_GENERATOR_CGIMSG_CLIENT";

const char* MessagePipe::kLockName = "message_pipe";

MessagePipe::MessagePipe(bool is_server) {
  is_server_ = is_server;
  lock_ = new InterProcLock(kLockName, true, is_server);

  block_ = NULL;
}

MessagePipe::~MessagePipe() {
  ReleaseResource();

  if (lock_ != NULL) delete lock_;
}

void MessagePipe::ReleaseResource() {
  shared_mem_.Destroy();
  block_ = NULL;

  owner_mutex_.Destroy();
  peer_mutex_.Destroy();
  mutex_set_.Destroy();
}

bool MessagePipe::ResetMutex() {
  if (!is_server_) {
    Logger::Log(EVENT_ERROR, "Client can't reset message pipe.");
    return false;
  }

  if (!Lock(kWaitTime)) {
    return false;
  }

  bool result = false;
  do {
    if (!peer_mutex_.Reset(false)) {
      Logger::Log(EVENT_ERROR, "Failed to reset peer mutex.");
      break;
    }
    if (!owner_mutex_.Reset(false)) {
      Logger::Log(EVENT_ERROR, "Failed to reset owner mutex.");
      break;
    }

    result = true;
  } while (false);

  Unlock();

  return result;
}

bool MessagePipe::Lock(int wait_ms) {
  if (!lock_->Lock(wait_ms)) {
    Logger::Log(EVENT_ERROR, "Failed to lock message pipe.");
    return false;
  }

  return true;
}

void MessagePipe::Unlock() {
  if (!lock_->Unlock()) {
    Logger::Log(EVENT_ERROR, "Failed to unlock message pipe.");
  }
}

bool MessagePipe::AcquireResource() {
  // Load mutex set.
  if (!mutex_set_.Load()) {
    Logger::Log(EVENT_ERROR, "Failed to load mutex set.");
    return false;
  }

  // Load mutexes.
  if (!mutex_set_.GetMutex(kClientMutexName, false, true,
                           is_server_? &peer_mutex_ : &owner_mutex_)) {
    Logger::Log(EVENT_ERROR, "Failed to get client mutex.");
    return false;
  }

  if (!mutex_set_.GetMutex(kServerMutexName, false, true,
                           is_server_? &owner_mutex_ : &peer_mutex_)) {
    Logger::Log(EVENT_ERROR, "Failed to get server mutex.");
    return false;
  }

  // Load shared memory.
  bool shared_mem_exist;
  if (!shared_mem_.Create(kSharedMemName, sizeof(MessageBlock),
                          is_server_, &shared_mem_exist)) {
    Logger::Log(EVENT_ERROR, "Failed to create shared memory.");
    return false;
  }

  block_ = reinterpret_cast<MessageBlock*>(shared_mem_.data());
  return true;
}

bool MessagePipe::Initialize() {
  // Initialize mutex set.
  if (!mutex_set_.Initialize("messagepipe_mutexset", is_server_)) {
    Logger::Log(EVENT_ERROR, "Failed to initialize mutex set.");
    return false;
  }
  mutex_set_.RegisterMutex(kServerMutexName);
  mutex_set_.RegisterMutex(kClientMutexName);

  // Try to acquire resource.
  if (!AcquireResource()) {
    Logger::Log(EVENT_IMPORTANT, "MessagePipe failed to acquire resource.");
    return false;
  } else {
    return true;
  }
}

bool MessagePipe::Send(const std::string& message) {
  Mutex::MutexResult result;
  int msg_len = static_cast<int>(message.length());
  int blocks = (msg_len - 1) / MessageBlock::kBlockSize + 1;
  for (int i = 0, j = 0; i < msg_len; i += MessageBlock::kBlockSize, ++j) {
    // Try to obtain owner mutex to write data.
    if (i != 0) {
      result = owner_mutex_.Wait(kWaitTime);
      if (result != Mutex::MUTEX_OK) {
        Logger::Log(EVENT_ERROR, "Message sender can't wait owner mutex. (%d|%d)",
                  result, owner_mutex_.LastError());
        return false;
      }
    }

    // Copy the data to shared memory.
    block_->length = msg_len - i;
    block_->block_index = j;
    block_->block_count = blocks;
    if (block_->length > MessageBlock::kBlockSize) {
      block_->length = MessageBlock::kBlockSize;
    }
    memcpy(block_->data, message.c_str() + i, block_->length);

    // Notify peer mutex to let peer receive data.
    result = peer_mutex_.Post();
    if (result != Mutex::MUTEX_OK) {
      Logger::Log(EVENT_ERROR, "Message sender can't post peer mutex. (%d|%d)",
                result, peer_mutex_.LastError());
      return false;
    }
  }

  return true;
}

bool MessagePipe::Receive(std::string* message) {
  Mutex::MutexResult result;
  message->assign("");
  int next_block = 0;
  while (true) {
    // Wait signal from sender.
    if (is_server_ && next_block == 0) {
      result = owner_mutex_.Wait(Mutex::kWaitInfinite);
    } else {
      result = owner_mutex_.Wait(kWaitTime);
    }
    if (result != Mutex::MUTEX_OK) {
      Logger::Log(EVENT_ERROR, "Message reciever can't wait rcv mutex. (%d|%d)",
                result, owner_mutex_.LastError());
      return false;
    }

    // Sanity check.
    if (block_->length > MessageBlock::kBlockSize || block_->length < 0) {
      Logger::Log(EVENT_ERROR, "Message block exceeds size.");
      return false;
    }
    if (next_block != block_->block_index) {
      Logger::Log(EVENT_ERROR, "Invalid block number [%d|%d].",
                next_block, block_->block_index);
      return false;
    }

    ++next_block;
    message->append(block_->data, block_->data + block_->length);

    // Make this block dirty.
    block_->block_index = -466453;

    if (next_block == block_->block_count) {
      break;
    }

    // Notify peer mutex to let peer know.
    result = peer_mutex_.Post();
    if (result != Mutex::MUTEX_OK) {
      Logger::Log(EVENT_ERROR, "Message receiver can't post peer mutex. (%d|%d)",
                result, peer_mutex_.LastError());
      return false;
    }
  }

  return true;
}
