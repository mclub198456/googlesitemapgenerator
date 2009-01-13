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


#include "common/criticalsection.h"

#ifdef WIN32

CriticalSection::CriticalSection() {
  InitializeCriticalSection(&criticalsection_);
}

CriticalSection::~CriticalSection() {
  DeleteCriticalSection(&criticalsection_);
}

bool CriticalSection::Enter(bool wait) {
  if (wait) {
    // Should always succeed.
    EnterCriticalSection(&criticalsection_);
    return true;
  } else {
    return TryEnterCriticalSection(&criticalsection_) != 0;
  }
}

void CriticalSection::Leave() {
  LeaveCriticalSection(&criticalsection_);
}

#elif defined(__linux__) || defined(__unix__)

CriticalSection::CriticalSection() {
  pthread_mutex_init(&mutex_, NULL);
}

CriticalSection::~CriticalSection() {
  pthread_mutex_destroy(&mutex_);
}

bool CriticalSection::Enter(bool wait) {
  if (wait) {
    return pthread_mutex_lock(&mutex_) == 0;
  } else {
    return pthread_mutex_trylock(&mutex_) == 0;
  }
}

void CriticalSection::Leave() {
  pthread_mutex_unlock(&mutex_);
}

#endif


AutoLeave::AutoLeave(CriticalSection *cs) {
  critical_section_ = cs;
  left_ = false;
}

void AutoLeave::LeaveAhead() {
  if (!left_) {
    critical_section_->Leave();
    left_ = true;
  }
}

AutoLeave::~AutoLeave() {
  LeaveAhead();
}
