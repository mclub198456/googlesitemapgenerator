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

//
// A website may have multiple host names. HostTable class is used to count
// the visiting times for each host name. A HostTable instance is expected to
// hold host names for the same website. Different website should have
// different HostTable.
// Besides the add/remove operations, this class also provides method to
// load/save host informations from/to file.
//
// Note, this class is not thread safe.

#ifndef SITEMAPSERVICR_HOSTTABLE_H__
#define SITEMAPSERVICR_HOSTTABLE_H__

#include <string>
#include <vector>

#include "common/hashmap.h"
#include "common/basictypes.h"
#include "common/url.h"

// Define type for host finger print.
typedef UrlFprint HostFprint;

// Structure used to hold host related information.
struct HostInfo {
  // The host name, like "www.exmaple.com".
  char name[kMaxHostLength];

  // The visiting times belonging to this host name.
  int visit_count;
};

// Table used to hold all HostInfo for a single site.
class HostTable {
  // Data structure used to hold HostInfo.
  typedef HashMap<HostFprint, HostInfo>::Type HashTable;

 public:
  // non-null, non-empty, unique website id is required.
  explicit HostTable(const char* siteid);

  ~HostTable();

  // Return id of website, to which all hosts belongs.
  const std::string& siteid() const { return siteid_; }

  // Get all host names contained in this table
  std::vector<HostInfo> GetAllHosts() const;

  // Visit specified "host" for "count" times. In other words,
  // the visting times of given "host" name is increased by "count".
  //
  // Returns the total visting count of "host"
  int VisitHost(const char* host, int count);

  // Get the most popular host name from this table.
  // The most popular host is the host which has the biggest visiting count.
  // If two host name has same visiting count, the result is undetermined.
  // An empty string would be returned when this table is empty.
  const std::string GetBestHost() const;

  // Remove specified "host" from this table.
  void RemoveHost(const char* host);

  // Get visiting count for specified "host".
  // -1 will be returned if there is no such host.
  int GetVisitCount(const char* host);

  // Get the number of hosts contained in this table.
  int Size() const { return static_cast<int>(hosts_.size()); }

  // Clear all hosts contained in this table.
  void Clear();

  // Write this table to a file.
  // Return whether saving is successful.
  bool Save(const char* path) const;

  // Load a table from file.
  // Return whether loading is successful.
  // NOTE, all old hosts in this table will be cleared.
  bool Load(const char* path);

 private:
  // Key: finger print of host name.
  // Value: the actual host information, which are pointers.
  // The pointers are owned by this table, and may be deleted in Clear() method.
  HashTable hosts_;

  // the id of a website, to which all the hosts in this table belongs.
  std::string siteid_;

  DISALLOW_EVIL_CONSTRUCTORS(HostTable);
};

#endif  // SITEMAPSERVICR_HOSTTABLE_H__


