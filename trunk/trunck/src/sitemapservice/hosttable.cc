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


#include "sitemapservice/hosttable.h"

HostTable::HostTable(const char* siteid)
  : siteid_(siteid) {
}

HostTable::~HostTable() {
  Clear();
}

std::vector<HostInfo> HostTable::GetAllHosts() const {
  // Iterate through the hash table to get host name.
  std::vector<HostInfo> infos;
  HashTable::const_iterator itr = hosts_.begin();
  for (; itr != hosts_.end(); ++itr) {
    infos.push_back(itr->second);
  }
  return infos;
}

int HostTable::VisitHost(const char* host, int count) {
  if (host == NULL || host[0] == '\0') return 0;

  // Try to find the corresponding table entry.
  HostFprint fprint = Url::FingerPrint(host);
  HashTable::iterator itr = hosts_.find(fprint);

  if (itr == hosts_.end()) {
    // Add a new table entry.
    HostInfo info;
    strncpy(info.name, host, kMaxHostLength - 1);
    info.name[kMaxHostLength-1] = '\0';
    info.visit_count = count;
    hosts_[fprint] = info;
    return count;
  } else {
    itr->second.visit_count += count;
    return itr->second.visit_count;
  }
}

int HostTable::GetVisitCount(const char* host) {
  HostFprint fprint = Url::FingerPrint(host);
  HashTable::iterator itr = hosts_.find(fprint);

  if (itr == hosts_.end()) {
    return -1;
  } else {
    return itr->second.visit_count;
  }
}

void HostTable::RemoveHost(const char* host) {
  HostFprint fprint = Url::FingerPrint(host);
  hosts_.erase(fprint);
}

void HostTable::Clear() {
  hosts_.clear();
}

// Writes this table to a file.
bool HostTable::Save(const char* path) const {
  FILE* file = fopen(path, "wb");
  if (file == NULL) return false;

  // Write hashtable entry to file.
  HashTable::const_iterator itr = hosts_.begin();
  for (; itr != hosts_.end(); ++itr) {
    if (fwrite(&(itr->second), sizeof(HostInfo), 1, file) != 1) {
      fclose(file);
      return false;
    }
  }

  fclose(file);
  return true;
}

// Loads a table from file.
bool HostTable::Load(const char* path) {
  FILE* file = fopen(path, "rb");
  if (file == NULL) return false;

  // Clear previous records first.
  Clear();

  // Read hostinfo from file and inserted it into hashtable.
  HostInfo info;
  while (fread(&info, sizeof(HostInfo), 1, file) == 1) {
    VisitHost(info.name, info.visit_count);
  }

  fclose(file);
  return true;
}

const std::string HostTable::GetBestHost() const {
  // Iterate through hashtable to find largest visiting count.
  HashTable::const_iterator itr = hosts_.begin(), best = hosts_.begin();
  for (; itr != hosts_.end(); ++itr) {
    if (best->second.visit_count < itr->second.visit_count) {
      best = itr;
    }
  }

  if (best != hosts_.end()) {
    return best->second.name;
  } else {
    return std::string();
  }
}

