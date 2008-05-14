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


#ifdef WIN32

#include "common/urlpipe.h"

#include <Aclapi.h>
#include "common/util.h"
#include "common/iisconfig.h"

// Constant names for the events and shared memory
const wchar_t* kShareMemName = L"GOOGLE_SITEMAP_GENERATOR_SHARE_MEM_NAME";
const wchar_t* kRwEventName = L"GOOGLE_SITEMAP_GENERATOR_RW_EVENT_NAME";
const wchar_t* kNotifyEventName = L"GOOGLE_SITEMAP_GENERATOR_NOTIFY_EVENT_NAME";

UrlPipe::UrlPipe(void) {
  rw_event_ = NULL;
  notify_event_ = NULL;
  file_mapping_ = NULL;
  records_ = NULL;
}

UrlPipe::~UrlPipe(void) {
  ReleaseResource();
}

void UrlPipe::ReleaseResource() {
  if (buffer_.GetInternalData() != NULL) {
    UnmapViewOfFile(file_mapping_);
  }


  if (file_mapping_ != NULL) {
    CloseHandle(file_mapping_);
    file_mapping_ = NULL;
  }

  if (notify_event_ != NULL) {
    CloseHandle(notify_event_);
    notify_event_ = NULL;
  }


  if (rw_event_ != NULL) {
    CloseHandle(rw_event_);
    rw_event_ = NULL;
  }

  if (records_ != NULL) {
    delete[] records_;
    records_ = NULL;
  }
}

bool UrlPipe::InitACL(PACL* ppacl) {
  // Retrieve security identifier for "Everyone".
  char sidbuffer[1024];
  PSID psid = reinterpret_cast<PSID>(sidbuffer);
  DWORD sidbuffer_size = 1024;

  char domainbuffer[1024];
  DWORD domainbuffer_size = 1024;

  SID_NAME_USE snu;
  if (!LookupAccountNameA(NULL, "Everyone",
                          psid, &sidbuffer_size,
                          domainbuffer, &domainbuffer_size,
                          &snu)) {
     Util::Log(EVENT_ERROR, "Failed to lookup Everyone account name. (%s).",
               GetLastError());
     return false;
  }

  // Build an ACL and allow Everyone's full control.
  // The memory must can be freed by "LocalFree" method.
  *ppacl = reinterpret_cast<PACL>(LocalAlloc(0, 2048));
  if (*ppacl == NULL) {
    Util::Log(EVENT_ERROR, "Failed to alloc memory for ACL. (%d).",
              GetLastError());
    return false;
  }

  if (!InitializeAcl(*ppacl, 1024, ACL_REVISION)) {
    Util::Log(EVENT_ERROR, "Failed to initialize ACL. (%d).", GetLastError());
    return false;
  }

  if (!AddAccessAllowedAce(*ppacl, ACL_REVISION, GENERIC_ALL, psid)) {
    Util::Log(EVENT_ERROR, "Failed to add permission for Everyone. (%d)",
              GetLastError());
    return false;
  }

  return true;
}

bool UrlPipe::Initialize(bool is_receiver) {
  // Do some role based initialization.
  is_receiver_ = is_receiver;
  if (is_receiver) {
    records_ = new UrlRecord[kUrlBufferSize];
  }

  // Event and Shared Memeory initialization code is same
  // for both receiver and senders.
  SECURITY_ATTRIBUTES sa;
  SECURITY_DESCRIPTOR sd;
  if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
    Util::Log(EVENT_ERROR, "Failed to initialize SD.");
    return false;
  }

  PACL pacl = NULL;
  if (!InitACL(&pacl)) {
    Util::Log(EVENT_ERROR, "Failed to init ACL.");
    return false;
  }

  // Set ACL to the Security Descriptor.
  if (SetSecurityDescriptorDacl(&sd, TRUE, pacl, FALSE) == FALSE) {
    Util::Log(EVENT_ERROR, "Failed to set Dacl to Security descriptor. 0x%x",
              GetLastError());
    if (pacl != NULL) LocalFree(pacl);
    return false;
  }

  sa.nLength = sizeof(sa);
  sa.lpSecurityDescriptor = &sd;
  sa.bInheritHandle = FALSE;

  // Initialize events and buffer with SA constructed above.
  bool suc = InitEvents(&sa) && InitBuffer(&sa);

  // Release resources.
  if (!suc) ReleaseResource();
  if (pacl != NULL) LocalFree(pacl);

  return suc;
}

bool UrlPipe::InitEvents(SECURITY_ATTRIBUTES* sa) {
  // Initialize rw_event_;
  rw_event_ = OpenEvent(EVENT_ALL_ACCESS, false, kRwEventName);
  if (rw_event_ == NULL) {
    rw_event_ = CreateEvent(sa, false, true, kRwEventName);
  }
  if (rw_event_ == NULL) {
    Util::Log(EVENT_ERROR, "Failed to create RW event. (%d)", GetLastError());
    return false;
  }

  // Initialize notify_event_;
  notify_event_ = OpenEvent(EVENT_ALL_ACCESS, false, kNotifyEventName);
  if (notify_event_ == NULL) {
    notify_event_ = CreateEvent(sa, false, false, kNotifyEventName);
  }
  if (notify_event_ == NULL) {
    Util::Log(EVENT_ERROR, "Failed to create Notify event. (%d)", GetLastError());
    return false;
  }

  return true;
}

bool UrlPipe::InitBuffer(SECURITY_ATTRIBUTES* sa) {
  HANDLE view_of_file = NULL;
  bool is_buffer_allocted = false;

  // Try to get handle of shared memory.
  file_mapping_ = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, kShareMemName);
  if (file_mapping_ == NULL) {
    file_mapping_ = CreateFileMapping(
      INVALID_HANDLE_VALUE,
      sa,
      PAGE_READWRITE,           // protection
      0, sizeof(UrlBufferData),
      kShareMemName);

    is_buffer_allocted = true;
  }

  if (file_mapping_ != NULL) {
    // Attach the shared memory.
    view_of_file = MapViewOfFile(file_mapping_, FILE_MAP_ALL_ACCESS, 0, 0, 0);

    if (view_of_file) {
      UrlBufferData* internaldata = static_cast<UrlBufferData*>(view_of_file);
      buffer_.SetInternalData(internaldata);
      if (is_buffer_allocted) {
        buffer_.Initialize();
      }
      return true;
    } else {
      Util::Log(EVENT_ERROR, "Failed to map view of file. (%d)",
                GetLastError());
      return false;
    }
  } else {
    Util::Log(EVENT_ERROR, "Failed to create file mapping. (%d)",
              GetLastError());
    return false;
  }
}

int UrlPipe::Send(UrlRecord *urlrecord, int count) {
  // Wait RW event to write buffer.
  DWORD result = 0;
  if (kSendWaitTime < 0) {
    result = WaitForSingleObject(rw_event_, INFINITE);
  } else {
    result = WaitForSingleObject(rw_event_, kSendWaitTime);
  }

  if (result == WAIT_OBJECT_0) {
    // Write record to buffer.
    int writecount = buffer_.WriteRecords(urlrecord, count);

    // Release RW event.
    if (SetEvent(rw_event_) == FALSE) {
      Util::Log(EVENT_NORMAL, "Sender can't set RW event. (%d)",
                GetLastError());
      return -1;
    }

    // Set NOTIFY event to notify receiver.
    if (SetEvent(notify_event_) == FALSE) {
      Util::Log(EVENT_NORMAL, "Sender can't set NOTIFY event. (%d)",
                GetLastError());
      return -1;
    }

    return writecount;
  } else if (result == WAIT_TIMEOUT) {
    Util::Log(EVENT_NORMAL, "Sender's waiting is time-out.");
    return 0;
  } else {
    Util::Log(EVENT_NORMAL, "Sender can't wait RW event. (%d)",
              GetLastError());
    return -1;
  }
}

int UrlPipe::Receive(UrlRecord** records) {
  // Wait for NOTIFY and RW event to get URL count in buffer.
  int count = 0;
  if (WaitForSingleObject(notify_event_, INFINITE) == WAIT_OBJECT_0) {
    if (WaitForSingleObject(rw_event_, INFINITE) == WAIT_OBJECT_0) {
      count = buffer_.GetRecordsCount();
      if (SetEvent(rw_event_) == FALSE) {
        Util::Log(EVENT_NORMAL, "Receiver can't set RW event-1 (%d)",
                  GetLastError());
        return -1;
      }
    } else {
      Util::Log(EVENT_NORMAL, "Reciever can't wait rw_event-1 (%d).",
                GetLastError());
      return -1;
    }
  } else {
    Util::Log(EVENT_NORMAL, "Receiver can't wait notify_event. (%d)",
              GetLastError());
    return -1;
  }

  // Read records from buffer.
  for (int i = 0; i < count; ++i) {
    UrlRecord* record = buffer_.GetRecord(i);
    if (record != NULL) {
      records_[i] = *record;
    } else {
      Util::Log(EVENT_NORMAL, "Receiver encounter invalid record.");
    }
  }

  // Move index in buffer to indicate the records have been read.
  if (WaitForSingleObject(rw_event_, INFINITE) == WAIT_OBJECT_0) {
    buffer_.ConsumeRecords(count);
    if (SetEvent(rw_event_) == FALSE) {
      Util::Log(EVENT_NORMAL, "Receiver can't set RW event-2 (%d).",
                GetLastError());
      return -1;
    }
  } else {
    Util::Log(EVENT_ERROR, "Receiver can't wait rw_event-2 (%d).",
              GetLastError());
    return -1;
  }

  *records = records_;
  return count;
}
#endif // WIN32


// Linux implementation.
#ifdef __linux__

#include <pthread.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <grp.h>

#include "common/urlpipe.h"
#include "common/util.h"
#include "common/fileutil.h"


union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

// Change the value of semnum in resource_.sem_id.
// It's a convenient wrapper of system "semctl" method.
int UrlPipe::SetSemValue(int semnum, int value) {
  semun semctl_arg;
  semctl_arg.val = value;
  int result = semctl(resource_.sem_id, semnum, SETVAL, semctl_arg);
  return result == -1 ? errno : 0;
}

// Run semaphore operation for the semnum in resource_.sem_id.
// It's a convenient wrapper of system "semop" method.
int UrlPipe::RunSemop(int num, int op, int flag, int wait_time) {
  sembuf operation;
  operation.sem_num = static_cast<short>(num);
  operation.sem_flg = static_cast<short>(flag);
  operation.sem_op = op;

  int result = -1;
  if (wait_time < 0) {
    result = semop(resource_.sem_id, &operation, 1);
  } else if (wait_time == 0) {
    operation.sem_flg |= IPC_NOWAIT;
    result = semop(resource_.sem_id, &operation, 1);
  } else {
    timespec timeout;
    timeout.tv_sec = wait_time / 1000;
    timeout.tv_nsec = (wait_time % 1000) * 1000000;
    result = semtimedop(resource_.sem_id, &operation, 1, &timeout);
  }

  return result == 0 ? 0 : errno;
}

UrlPipe::UrlPipe() {
  records_ = NULL;

  resource_.sem_id = -1;
  resource_.shm_id = -1;

  lock_dir_ = "/var/lock/google-sitemap-generator";

  buffer_.SetInternalData(NULL);
}

UrlPipe::~UrlPipe() {
  ReleaseResource();

  if (records_ != NULL) {
    delete[] records_;
  }
}

// initialize semophore and shared memory
bool UrlPipe::Initialize(bool is_receiver) {
  if (!FileUtil::CreateDir(lock_dir_.c_str())) {
    Util::Log(EVENT_ERROR, "Failed to create lock_dir_.");
    return false;
  }

  is_receiver_ = is_receiver;
  if (is_receiver) {
    records_ = new UrlRecord[kUrlBufferSize];
    return CreateResource();
  } else {
    if (!RetrieveResource()) {
      Util::Log(EVENT_ERROR, "Failed to retrieve resource, try again later.");
    }
    return true;
  }
}

bool UrlPipe::CreateResource() {
  // Open lock_sem file.
  std::string lock_file(lock_dir_);
  lock_file.append("/urlpipe.lck");
  int lockfd = open(lock_file.c_str(),
                    O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  if (lockfd == -1) {
    Util::Log(EVENT_ERROR, "Failed to open lock_sem (%d).", errno);
    return false;
  }

  // Try to lock lock_sem file.
  if (flock(lockfd, LOCK_EX) != 0) {
    Util::Log(EVENT_ERROR, "Failed to lock_sem. (%d).", errno);
    close(lockfd);
    return false;
  }

  bool final_result = false;
  do {
    // Obtain a unique resource_.sem_id.
    resource_.sem_id = semget(IPC_PRIVATE, 2,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | IPC_CREAT);
    if (resource_.sem_id < 0) {
      Util::Log(EVENT_ERROR, "Failed to get private sem id. (%d)", errno);
      break;
    }

    // Obtain a unique resource_.shm_id.
    resource_.shm_id = shmget(IPC_PRIVATE, sizeof(UrlBufferData),
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | IPC_CREAT);
    if (resource_.shm_id < 0) {
      Util::Log(EVENT_ERROR, "Failed to get private shm id. (%d)", errno);
      break;
    }

    // Initialize the sem numbers.
    if (SetSemValue(kRwSemNum, 1) == -1) {
      Util::Log(EVENT_ERROR, "Failed to initialize RW sem (%d)", errno);
      break;
    }

    if (SetSemValue(kNotifySemNum, 0) == -1) {
      Util::Log(EVENT_ERROR, "Failed to initialize NOTIFY sem (%d)", errno);
      break;
    }

    // Attach shared memory.
    void* shm_pointer = shmat(resource_.shm_id, 0, 0);
    if (shm_pointer == reinterpret_cast<void*>(-1)) {
      Util::Log(EVENT_ERROR, "UrlPipe can't attach shm (%d)", errno);
      break;
    }
    buffer_.SetInternalData(static_cast<UrlBufferData*>(shm_pointer));
    buffer_.Initialize();  // reset buffer whenever initialized

    // Save it to file.
    if (write(lockfd, &resource_, sizeof(Resource)) < 0) {
      Util::Log(EVENT_ERROR, "Failed to save resource id. (%d)", errno);
      break;
    }

    Util::Log(EVENT_IMPORTANT, "Resource[%x] is created: sem-%d, shm-%d.",
              this, resource_.sem_id, resource_.shm_id);

    // All done.
    final_result = true;
  } while (false);

  flock(lockfd, LOCK_UN);
  close(lockfd);

  return final_result;
}

bool UrlPipe::RetrieveResource() {
  time(&last_retrieve_);

  // Open lock file.
  std::string lock_file(lock_dir_);
  lock_file.append("/urlpipe.lck");
  int lockfd = open(lock_file.c_str(), O_RDONLY);
  if (lockfd == -1) {
    Util::Log(EVENT_ERROR, "Failed to open resource file (%d).", errno);
    return false;
  }

  // Try to lock Lock file.
  if (flock(lockfd, LOCK_EX) != 0) {
    Util::Log(EVENT_ERROR, "Failed to lock resource file. (%d).", errno);
    close(lockfd);
    return false;
  }

  bool final_result = false;
  do {
    // Try to read resource ids.
    ssize_t readsize = read(lockfd, &resource_, sizeof(Resource));
    if (readsize < 0) {
      Util::Log(EVENT_ERROR, "Failed to read resource lock file. (%d).", errno);
      break;
    } else if (readsize != sizeof(Resource)) {
      Util::Log(EVENT_ERROR, "The resource lock file is bad formatted.");
      break;
    }

    // Attach shared memory.
    void* shm_pointer = shmat(resource_.shm_id, 0, 0);
    if (shm_pointer == reinterpret_cast<void*>(-1)) {
      Util::Log(EVENT_ERROR, "UrlPipe can't attach shm (%d)", errno);
      break;
    }
    buffer_.SetInternalData(static_cast<UrlBufferData*>(shm_pointer));

    Util::Log(EVENT_IMPORTANT, "Resource[%x] is retrieved: sem-id:%d, shm-id:%d",
              this, resource_.sem_id, resource_.shm_id);

    final_result = true;
  } while (false);

  // Reset the value to indicate no resource is retrieved.
  if (final_result == false) {
    resource_.shm_id = -1;
    resource_.sem_id = -1;
  }

  flock(lockfd, LOCK_UN);
  close(lockfd);
  return final_result;
}

void UrlPipe::ReleaseResource() {
  if (buffer_.GetInternalData() != NULL) {
    if (shmdt(buffer_.GetInternalData()) == -1) {
      Util::Log(EVENT_ERROR, "Failed to detach shm, ignore.(%d)", errno);
    }
    buffer_.SetInternalData(NULL);
  }

  // For receiver, the resources should be destroyed explicitly.
  if (is_receiver_) {
    if (resource_.sem_id != -1) {
      semun cmd;
      if (semctl(resource_.sem_id, 0, IPC_RMID, cmd) == -1) {
        Util::Log(EVENT_ERROR, "Failed to remove sem, ignore.(%d)", errno);
      }
    }

    if (resource_.shm_id != -1) {
      if (shmctl(resource_.shm_id, IPC_RMID, 0) == -1) {
        Util::Log(EVENT_ERROR, "Failed to remove shm, ignore.(%d)", errno);
      }
    }

    // Remove the lock file.
    std::string lock_file(lock_dir_);
    lock_file.append("/urlpipe.lck");
    remove(lock_file.c_str());
  }

  resource_.sem_id = -1;
  resource_.shm_id = -1;

  Util::Log(EVENT_IMPORTANT, "Resource[%x] is released[%d] sem:%d, shm:%d.",
            this, (is_receiver_ ? 1 : 0), resource_.sem_id, resource_.shm_id);
}

int UrlPipe::Send(UrlRecord *urlrecord, int count) {
  int result = 0;
  do {
    // Try to obtain RW semophore
    result = RunSemop(kRwSemNum, -1, SEM_UNDO, kSendWaitTime);
    if (result != 0) {
      Util::Log(EVENT_NORMAL, "Sender can't decrease RW sem (%d)", result);
      break;
    }

    int writecount = buffer_.WriteRecords(urlrecord, count);

    // Release RW sem.
    result = RunSemop(kRwSemNum, 1, SEM_UNDO, -1);
    if (result != 0) {
      Util::Log(EVENT_NORMAL, "Sender can't increase RW sem (%d)", result);
      break;
    }

    // Increase notify sem.
    result = SetSemValue(kNotifySemNum, 1);
    if (result != 0) {
      Util::Log(EVENT_NORMAL, "Sender can't set NOTIFY sem (%d)", result);
      break;
    }

    return writecount;
  } while (false);

  // Receiver may re-enter, so the receiver should renew its IPC objects.
  if (result == EIDRM) {
    time_t now = time(NULL);
    if (last_retrieve_ + kRetrievePeriod < now) {
      Util::Log(EVENT_IMPORTANT, "Sender tries to retrieve IPC objects again.");
      ReleaseResource();
      RetrieveResource();
      last_retrieve_ = now;
    }
    return 0;
  } else if (result == EAGAIN) {
    Util::Log(EVENT_NORMAL, "Sender's waiting is time-out.");
    return 0;
  }

  return -1;
}

int UrlPipe::Receive(UrlRecord** records) {
  // Wait for notify sem.
  int result = RunSemop(kNotifySemNum, -1, 0, -1);
  if (result != 0) {
    Util::Log(EVENT_NORMAL, "Receiver can't wait notify sem (%d)", result);
    return -1;
  }

  // Get RW sem to gets the record count.
  // Release the sem imediately after getting the count.
  result = RunSemop(kRwSemNum, -1, SEM_UNDO, -1);
  if (result != 0) {
    Util::Log(EVENT_NORMAL, "Receiver can't decrease sem-1 (%d)", result);
    return -1;
  }

  int count = buffer_.GetRecordsCount();

  result = RunSemop(kRwSemNum, 1, SEM_UNDO, -1);
  if (result != 0) {
    Util::Log(EVENT_NORMAL, "Receiver can't increase sem-1 (%d)", result);
    return -1;
  }

  // Store the records.
  for (int i = 0; i < count; ++i) {
    UrlRecord* record = buffer_.GetRecord(i);
    if (record != NULL) {
      records_[i] = *record;
    } else {
      Util::Log(EVENT_NORMAL, "Receiver encounter invalid record.");
    }
  }
  *records = records_;

  // Get RW sem to update buffer.
  result = RunSemop(kRwSemNum, -1, SEM_UNDO, -1);
  if (result != 0) {
    Util::Log(EVENT_NORMAL, "Receiver can't decrease RW sem-2 (%d)", result);
    return -1;
  }

  buffer_.ConsumeRecords(count);

  result = RunSemop(kRwSemNum, 1, SEM_UNDO, -1);
  if (result != 0) {
    Util::Log(EVENT_NORMAL, "Receiver can't increase RW sem-2 (%d)", result);
    return -1;
  }

  return count;
}

#endif // __linux__
