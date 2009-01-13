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


// This class is a thread which receives UrlRecord from UrlPipe.
// It waits on the receiver side of UrlPipe in blocking mode.
// The url records received may belong to different site. After receiving
// the records, this thread dispatchs the records to corresponding site's
// data manager.

#ifndef SITEMAPSERVICE_URLRECEIVETHREAD_H__
#define SITEMAPSERVICE_URLRECEIVETHREAD_H__

#include <string>
#include <map>
#include <time.h>

#include "common/thread.h"
#include "common/urlpipe.h"
#include "sitemapservice/sitemanager.h"
#include "sitemapservice/webserverfilterinfo.h"

class UrlReceiveThread : public Thread {
 public:
  UrlReceiveThread() {}
  virtual ~UrlReceiveThread() {}

  // Initialize the thread with settings and data managers.
  bool Initialize();

  // Unload old site.
  void RemoveSite(const std::string& site_id);

  // Load new site settings.
  // This class doesn't release the SiteManager pointer, and uses the
  // manager till this class dies.
  void AddSite(SiteManager* manager);

  // Run this thread.
  // The new thread would wait on the receiver side of UrlPipe to get records
  // from it.
  virtual void Run();

 private:
  // Represents configurations for a site.
  struct SiteEntry {
    SiteManager* site_manager;
    WebServerFilterInfo* filter_info;
    int64 urls_count;
  };

  // Process records.
  // This method is used in "Run" after records are recieved from UrlPipe.
  int ProcessRecords(UrlRecord *records, int count);

  // Maps a site-id to a SiteEntry.
  std::map<std::string, SiteEntry> sites_;

  // Used to lock sites_.
  CriticalSection sites_lock_;

  // UrlPipe used to commonicate URL records with webserver filter.
  UrlPipe pipe_;

  // The last update time of runtime information.
  time_t last_update_info_;
};

#endif // SITEMAPSERVICE_URLRECEIVETHREAD_H__
