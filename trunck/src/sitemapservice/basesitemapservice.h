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


// BaseSitemapService is the base class for all kinds of sitemap services.
// It defines some helper methods for the sub-classes to use.
// "StartGenerating", "AddUrl", "EndGenerating" is a group of methods used to
// create sitemap file. The subclasses can feed these methods with UrlElement.
// These methods can then generate a single sitemap file or sitemap index file
// according to the number of UrlElement.
// This class neither filter URLs nor submit the sitemap to search engine, but
// it provides "FilterUrl" and "InformSearchEngine" methods. The sub-classes
// should call them explicitly if they want such functionality.
// "Run" method is implemented by this class with runtime information support.
// Sub-classes are freed from RuntimeInformation, but they must implement
// "InternalRun" template method.
// This class is thread-safe after it is initialized.

#ifndef SITEMAPSERVICE_BASESITEMAPSERVICE
#define SITEMAPSERVICE_BASESITEMAPSERVICE

#include <string>

#include "common/basictypes.h"
#include "sitemapservice/sitemapelement.h"
#include "sitemapservice/serviceinterface.h"
#include "sitemapservice/urlfilter.h"
#include "sitemapservice/informer.h"
#include "sitemapservice/sitedatamanager.h"

class SitemapWriter;
class SitemapServiceInfo;

class BaseSitemapService : public ServiceInterface {

 public:
  // Constructor.
  // The "writer" pointer will be freed by this class.
  BaseSitemapService(SitemapWriter* writer, std::string sitemap_name);

  virtual ~BaseSitemapService();

  // Initialize with datamanager and setting.
  // This method simply stores the given parameters.
  virtual bool Initialize(SiteDataManager* datamanager, const SiteSetting& setting);

  // Get waiting time to run service next time.
  // The value is (CurrentTime - LastRunningTime).
  virtual int GetWaitTime();

  // Get running period.
  // The value is the "update_duration" value of sitemap setting.
  virtual int GetRunningPeriod();

  // Run the service.
  // This method does two things. One is to delegate service task to
  // InternalRun method. The other is to update runtime information according
  // to the internal state after InternalRun.
  // Sub-classes must implement InternalRun to ensure this service behave well.
  virtual void Run();

 protected:
  // This is a method that must be invoked by sub-classes. It builds include,
  // exclude filters according to the sitemapsettings. Sub-classes can call
  // FilterUrl to use these filters. This method also generate an empty sitemap
  // according to the sitemapsetting.
  // Sub-classes should set the specific sitemap setting and service info.
  bool Initialize(const SitemapSetting* sitemapsetting,
                  SitemapServiceInfo* sitemapservice_info);

  // Template method must be implemented by sub-classes.
  // Usually, it should call StartGenerating, AddUrl, and EndGenerating to
  // generate sitemap files. During this process, it may also call FilterUrl
  // to exclude some undesired URLs. After generating the sitemap, it could
  // call InformSearchEngine to submit sitemap files.
  // Anyway, it is all up to the sub-classes how to implement this method.
  virtual bool InternalRun() = 0;

  // Start to generate sitemap and initialize the generating process.
  // This method must be called before AddUrl method.
  // "host" is the host name used in result sitemap files.
  void StartGenerating(const char* host);

  // Add a Url to this generator.
  bool AddUrl(const UrlElement& url);

  // Ends generating process.
  // If alwaysindex is true, index file would be created even there is only one
  // sitemap file. And if noindex is true, no index file is created even there
  // are multiple sitemap file. In this case, only the first sitemap is kept.
  // And of course, these two values can't be true simutaneously.
  bool EndGenerating(bool alwaysindex, bool noindex);

  // Submit sitemap files to search engine.
  // The way to submit sitemap is defined in sitemap setting.
  void InformSearchEngine();

  // Filter url according to included filter and excluded filter.
  // Return true if the url is acceptable.
  bool FilterUrl(const char* url);

  // Useful variables to generate sitemap files.
  const SitemapSetting* sitemapsetting_;
  SiteSetting sitesetting_;
  SiteDataManager* data_manager_;
  std::string sitemap_name_;

 private:
  // Flushes all the cached urls to a new sitemap file.
  // It writes current left urls to a new sitemap file.
  // urlset_ will be cleared and a new entry will be added to sitemap_index_
  bool Flush();

  // Compress the given file
  // A gziped file with .gz suffix will be created,
  // and original file will be deleted
  bool CompressFile(const std::string& file);

  // Build a sitemap file path according to the given index
  // If index is negative, the final sitemap file name will be created.
  std::string BuildPath(int index, bool fullpath);

  // Indicates max sitemap included in index file.
  // This vlaue is defined by sitemap protocol in http://sitemaps.org
  static const int kMaxSitemapIndex = 1000;
 
  // service last run time
  time_t last_run_;

  // Base url of current generating process
  // It will be set in StartGenerating method.
  std::string host_;

  // Normalized physical directory to store sitemap.
  // It will be set in Initialize method.
  std::string directory_;

  // url set containing all the urls which haven't been written to file.
  UrlSetElement urlset_;

  // sitemap index containing all the generated sitemap files.
  SitemapIndexElement sitemap_index_;

  // used to write sitemap or sitemap index to file.
  // It will be set in Initialize method.
  SitemapWriter* writer_;
  
  UrlFilter* includefilter_;

  UrlFilter* excludefilter_;

  std::vector<Informer*> informers_;

  // Variables to hold runtime infos.
  // The values will be put to sitemapservice_info_ in SaveRuntimeInfo method.
  int urls_count_;

  // The run time info structure for the service.
  SitemapServiceInfo* sitemapservice_info_;

  DISALLOW_EVIL_CONSTRUCTORS(BaseSitemapService);
};

#endif // SITEMAPSERVICE_BASESITEMAPSERVICE




