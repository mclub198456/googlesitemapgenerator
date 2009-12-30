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


#include "sitemapservice/basesitemapservice.h"

#include <errno.h>
#include <time.h>
#include <assert.h>
#include <string>

#include "common/logger.h"
#include "common/util.h"
#include "common/fileutil.h"
#include "common/port.h"
#include "common/accesscontroller.h"
#include "sitemapservice/sitemapwriter.h"
#include "sitemapservice/urlfilterbuilder.h"
#include "sitemapservice/runtimeinfomanager.h"

BaseSitemapService::BaseSitemapService(SitemapWriter* writer,
                                       std::string sitemap_name) {
  includefilter_ = NULL;
  excludefilter_ = NULL;
  data_manager_ = NULL;
 
  assert(writer != NULL); 
  writer_ = writer;
  sitemap_name_ = sitemap_name;
  time(&last_run_);
}

BaseSitemapService::~BaseSitemapService() {
  if (includefilter_ != NULL) {
    delete includefilter_;
  }
  if (excludefilter_ != NULL) {
    delete excludefilter_;
  }

  if (writer_ != NULL) {
    delete writer_;
  }

  // Release all informers.
  for (int i = 0; i < static_cast<int>(informers_.size()); ++i) {
    delete informers_[i];
  }
}

bool BaseSitemapService::Initialize(SiteDataManager* datamanager,
                                    const SiteSetting& setting) {
  sitesetting_ = setting;
  data_manager_ = datamanager;

  // Change the add_generator_info flag in sitemap writer.
  writer_->SetAddGeneratorInfo(setting.add_generator_info());

  // normalize directory_, ensure it is ended by '/'
  directory_ = setting.physical_path();
  char last_char = directory_[directory_.length() - 1];
  if (last_char != '\\' && last_char != '/') {
    directory_.append("/");
  }

  return true;
}

bool BaseSitemapService::Initialize(const SitemapSetting* sitemapsetting,
                                    SitemapServiceInfo* sitemapservice_info) {
  sitemapsetting_ = sitemapsetting;
  sitemapservice_info_ = sitemapservice_info;

  // Build url filters.
  includefilter_ = UrlFilterBuilder::Build(
    UrlSetting::GetEnabledUrls(sitemapsetting_->included_urls().items()));
  excludefilter_ = UrlFilterBuilder::Build(
    UrlSetting::GetEnabledUrls(sitemapsetting_->excluded_urls().items()));

  // Initialize informers.
  std::vector<Url> urls = UrlSetting::GetEnabledUrls(
    sitemapsetting_->notify_urls().items());
  for (std::vector<Url>::const_iterator itr = urls.begin();
    itr != urls.end(); ++itr) {
    // here we assume informer will always be constructed successfully.
    informers_.push_back(new HttpRequestInformer(*itr));
  }

  // Mimic the generating process to generate an empty sitemap.
  // Note, dummystat and "for initialization" will not be used actually.
  // Because there is no url record involved in the generating process.
  std::string sitemapfile = BuildPath(-1, true);
  if (sitemapsetting_->compress()) sitemapfile.append(".gz");
  if (!FileUtil::Exists(sitemapfile.c_str())) {
    Logger::Log(EVENT_IMPORTANT, "Empty [%s] is generated for initialization.",
              sitemapfile.c_str());
    StartGenerating("for initialization");
    EndGenerating(false, true);
  }

  return true;
}

void BaseSitemapService::Run() {
  Logger::Log(EVENT_CRITICAL, "Start to generate [%s] for site [%s].",
    sitemap_name_.c_str(), sitesetting_.site_id().c_str());

  // Reset runtime info.
  urls_count_ = 0;

  // Run the service.
  bool result = InternalRun();
  if (result) {
    Logger::Log(EVENT_CRITICAL, "Succeeded to generate [%s] for site [%s].",
      sitemap_name_.c_str(), sitesetting_.site_id().c_str());
  } else {
    Logger::Log(EVENT_ERROR, "Failed to generate [%s] for site [%s].",
      sitemap_name_.c_str(), sitesetting_.site_id().c_str());
  }

  // Update last_run value.
  time(&last_run_);

  // Update runtime information.
  if (sitemapservice_info_ == NULL) return;

  if (RuntimeInfoManager::Lock(true)) {
    sitemapservice_info_->set_success(result);
    sitemapservice_info_->set_last_update(time(NULL));
    sitemapservice_info_->set_urls_count(urls_count_);
    RuntimeInfoManager::Unlock();
  }

}

void BaseSitemapService::StartGenerating(const char* host) {
  // Save the host value for later use.
  host_ = host;

  // Clear value left by previous process
  urlset_.Clear();
  sitemap_index_.Clear();
  urls_count_ = 0;
}

int BaseSitemapService::GetRunningPeriod() {
  return sitemapsetting_->update_duration();
}

int BaseSitemapService::GetWaitTime() {
  const time_t& start = sitemapsetting_->update_start_time();

  // It is not started yet.
  time_t now = time(NULL);
  if (now < start) {
    return static_cast<int>(start - now);
  }

  // Generate sitemap file whenever possible.
  if (sitemapsetting_->update_duration() <= 0) {
    return 0;
  }

  // Now is in the range of [expected_last, expected_next]
  time_t expected_last = now - (now - start) % sitemapsetting_->update_duration(); 
  time_t expected_next = expected_last + sitemapsetting_->update_duration();
  if (expected_last > last_run_) {
    return 0;
  } else {
    return static_cast<int>(expected_next - now);
  }
}

// Add url to urlset_ and write it to sitemap file if the urlset_ is full.
bool BaseSitemapService::AddUrl(const UrlElement &url) {
  ++urls_count_;
  urlset_.AddUrl(url);

  // Url number meets limit, so they should be written to a file
  if (sitemapsetting_->max_file_url_number() > 0
      && urlset_.Size() >= sitemapsetting_->max_file_url_number()) {
    return Flush();
  }

  return true;
}

// End generation of sitemap files.
// All URLs in urlset_ will be flushed to disk, and sitemap index file will
// be generated if necessary.
bool BaseSitemapService::EndGenerating(bool alwaysindex, bool noindex) {
  if (alwaysindex && noindex) {
    Logger::Log(EVENT_ERROR, "always-index and always-no-index conflicted.");
    return false;
  }

  // A flag indicating whether there is no url given by AddUrl.
  bool no_url = sitemap_index_.Size() == 0 && urlset_.Size() == 0;

  // Write all the left urls to sitemap file,
  // or if there is no url processed, and always index is false,
  // an empty sitemap should be generated.
  if (urlset_.Size() > 0 || (no_url && !alwaysindex)) {
    // Maybe we should write more than one time to flush all values.
    do {
      if (!Flush()) {
        return false;
      }
    } while (urlset_.Size() > 0);
  }

  // The final sitemap file name, which should be used to inform search engine.
  std::string final_file = BuildPath(-1, true);

  if ((sitemap_index_.Size() > 1 && noindex == false) || alwaysindex) {
    // A sitemap index file should be generated.

    // Compress all sitemap files.
    if (sitemapsetting_->compress()) {
      for (int i = 0; i < sitemap_index_.Size(); ++i) {
        std::string sitemap_file = BuildPath(i, true);
        CompressFile(sitemap_file.c_str());
      }
    }

    // Create the sitemap index file with "final_file" as its name.
    int result = writer_->WriteSitemapIndex(final_file, sitemap_index_);
    if (result == 0) {
      if (sitemapsetting_->compress()) {
        return CompressFile(final_file);
      } else {
        return true;
      }
    } else {
      Logger::Log(EVENT_ERROR, "Failed to create sitemap index [%s] for [%s].",
        final_file.c_str(), sitesetting_.site_id().c_str());
      return false;
    }

  } else if ((sitemap_index_.Size() == 1 && alwaysindex == false) || noindex) {
    // No sitemap index file should be generated.

    // When there is more than one sitemap, only sitemap-0 should be saved.
    if (sitemap_index_.Size() > 1) {
      for (int i = 1; i < sitemap_index_.Size(); ++i) {
        std::string sitemap_i = BuildPath(i, true);
        remove(sitemap_i.c_str());
      }
    }

    // Rename sitemap-0 to the final sitemap file name.
    std::string sitemap0 = BuildPath(0, true);
    remove(final_file.c_str());  // Try to remove destination file.
    if (rename(sitemap0.c_str(), final_file.c_str()) != 0) {
      Logger::Log(EVENT_ERROR, "Failed to rename file [%s] to [%s] (%d)\n",
                sitemap0.c_str(), final_file.c_str(), errno);
      return false;
    }

    // Compress the final sitemap file.
    if (sitemapsetting_->compress()) {
      return CompressFile(final_file);
    } else {
      return true;
    }
  } else {
    // Logic never comes here. Just for compiler.
    return true;
  }
}

bool BaseSitemapService::Flush() {
  // Check the sitemap limitation.
  if (sitemap_index_.Size() >= kMaxSitemapIndex) {
    Logger::Log(EVENT_ERROR, "Number of indexed sitemap exceeds 1000. Ignore.");
    urlset_.Clear();
    return true;
  }

  // Write urlset_ into a sitemap file.
  std::string next_file = BuildPath(sitemap_index_.Size(), false);
  std::string full_path = directory_ + next_file;
  int count = writer_->WriteSitemap(full_path, urlset_,
    sitemapsetting_->max_file_size());
  
  // IO operation failed.
  if (count == -1) {
    Logger::Log(EVENT_ERROR, "Failed to write sitemap file [%s].",
      full_path.c_str());
    return false; 
  }

  if (count == 0 && urlset_.Size() != 0) {
    Logger::Log(EVENT_ERROR, "Sitemap file size limit is too small.");
    return false;
  }

  // Remove the URLs which has been written to file.
  urlset_.RemoveRange(0, count);

  // Save the sitemap file path to sitemap_index_
  std::string sitemapurl(host_);
  sitemapurl.append("/").append(next_file);
  if (sitemapsetting_->compress()) {
    sitemapurl.append(".gz");
  }

  SitemapElement sitemap(sitemapurl, std::time(NULL));
  sitemap_index_.AddSitemap(sitemap);

  return true;
}

bool BaseSitemapService::CompressFile(const std::string& file) {
  // Build the compressed file name.
  std::string compressed(file);
  compressed.append(".gz");

  if (Util::GZip(file.c_str(), compressed.c_str())) {
    // Remove uncompressed file, ignore error.
    remove(file.c_str());
    if (!AccessController::AllowWebserverAccess(compressed,
                                                AccessController::kAllowRead)) {
      Logger::Log(EVENT_ERROR, "Writer can't chmod: %s.", compressed.c_str());
    }
    return true;
  } else {
    Logger::Log(EVENT_ERROR, "Failed to compress file: [%s]. (%d)",
              file.c_str(), errno);
    return false;
  }
}

// Build sitemap file path.
std::string BaseSitemapService::BuildPath(int index, bool fullpath) {
  // Build index string, like "_000", "_001", ..., "_1234567"...
  // Actually, the given index can never exceed 999.
  std::string index_str = "";
  if (index >= 0) {
    char buffer[256];
    itoa(index, buffer);
    index_str.append(buffer);

    // Make it 3 digit width.
    if (index_str.length() < 3) {
      index_str.insert(0, 3 - index_str.length(), '0');
    }
    index_str.insert(0, 1, '_');
  }

  // Insert index_str into filename.
  std::string filename = sitemapsetting_->file_name();
  std::string::size_type pos = filename.find_last_of(".");
  if (pos != std::string::npos) {
    filename.insert(pos, index_str);
  } else {
    filename.append(index_str);
  }

  if (fullpath) {
    filename.insert(0, directory_);
  }

  return filename;
}

bool BaseSitemapService::FilterUrl(const char* url) {
  int urllen = static_cast<int>(strlen(url));
  if (includefilter_ == NULL || !includefilter_->Accept(url, urllen)) {
    // Not an included url.
    return false;
  }
  if (excludefilter_ != NULL && excludefilter_->Accept(url, urllen)) {
    // An excluded url.
    return false;
  }

  return true;
}

void BaseSitemapService::InformSearchEngine() {
  std::string sitemap_path = BuildSitemapUrl();
  // Notify every informers
  std::vector<Informer*>::iterator itr = informers_.begin();
  for ( ; itr != informers_.end(); ++itr) {
    (*itr)->Inform(sitemap_path);
  }
}

std::string BaseSitemapService::BuildSitemapUrl() {
    // Construct the url of sitemap, like:
  // http://www.example.com/sitemap.xml
  std::string sitemap_path(host_);
  sitemap_path.append("/").append(sitemapsetting_->file_name());
  if (sitemapsetting_->compress()) {
    sitemap_path.append(".gz");
  }
  return sitemap_path;
}
