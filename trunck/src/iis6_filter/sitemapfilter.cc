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


// Remove url filtering functionality from this class.
// Rewrite the code to make it thread-safe.

// Add the function to install and uninstall filter

#include "common/version.h"
#include "common/sitesettings.h"
#include "common/util.h"
#include "common/fileutil.h"

#include "iis6_filter/sitemapfilter.h"


bool SitemapFilter::Initialize(HMODULE module_handle) {
  Util::Log(EVENT_CRITICAL, "=== Google Sitemap Generator IIS Filter [%s] ===",
            SITEMAP_VERSION1);

  // Get setting file path.
  std::string module_dir;
  if (!Util::GetModuleDir(module_handle, &module_dir)) {
    Util::Log(EVENT_ERROR, "Failed to get sitemap filter dll dir.");
    return false;
  }
  std::string settings_file = SiteSettings::GetDefaultFilePath(module_dir);

  // Load settings.
  SiteSettings settings;
  if (!settings.LoadWebserverConfig()) {
    Util::Log(EVENT_ERROR, "Failed to load webserver config.");
    return false;
  }
  if (!settings.LoadFromFile(settings_file.c_str())) {
    Util::Log(EVENT_ERROR, "Failed to load setting from file.");
    return false;
  }

  // Initialize flags.
  Util::SetLogLevel(settings.logging_level());

  return BaseFilter::Initialize(settings);
}

bool SitemapFilter::GetFilterVersion(HTTP_FILTER_VERSION * filter_version) {
  // Specify the types and order of notification.
  filter_version->dwFlags = SF_NOTIFY_SECURE_PORT |
                            SF_NOTIFY_NONSECURE_PORT |
                            SF_NOTIFY_SEND_RESPONSE |
                            SF_NOTIFY_ORDER_DEFAULT; 

  filter_version->dwFilterVersion = HTTP_FILTER_REVISION; 

  // Set filter description like "Google Sitemap Filter 0.0"
  std::string filter_description = "Google Sitemap Filter";
  filter_description.append(" ").append(SITEMAP_VERSION1);
  strncpy(filter_version->lpszFilterDesc, 
          filter_description.c_str(),
          SF_MAX_FILTER_DESC_LEN); 

  return true;
}


DWORD SitemapFilter::Process(HTTP_FILTER_CONTEXT *context, 
                             DWORD notification_type, 
                             VOID *data) {  
  // Buffer used to recieve http variables/parameters.
  static const int kBufferSize = 1024;
  char buffer[kBufferSize];
  DWORD size = kBufferSize;
  BOOL suc = FALSE;

  if (notification_type & SF_NOTIFY_SEND_RESPONSE) {

    // Only "GET" method is supported.
    size = kBufferSize;
    suc = context->GetServerVariable(context, "HTTP_METHOD", buffer, &size);
    if (!suc || buffer[0] != 'G') {
      return SF_STATUS_REQ_NEXT_NOTIFICATION;
    }

    UrlRecord record;

    // Check the http status code.
    HTTP_FILTER_SEND_RESPONSE* sendresponse = static_cast<HTTP_FILTER_SEND_RESPONSE*>(data);
    if (!BaseFilter::CheckStatusCode(sendresponse->HttpStatus)) {
      return SF_STATUS_REQ_NEXT_NOTIFICATION;
    }
    record.statuscode = sendresponse->HttpStatus;

    // Check the site id to see whether it is enabled.
    size = kMaxSiteIdLength;
    suc = context->GetServerVariable(context, "INSTANCE_ID", record.siteid, &size);
    if (!suc) return SF_STATUS_REQ_NEXT_NOTIFICATION;
    int siteindex = BaseFilter::MatchSite(record.siteid);
    if (siteindex == -1) return SF_STATUS_REQ_NEXT_NOTIFICATION;

    // Save the protocol to reocrd.host.
    int offset = 0;
    if (notification_type & SF_NOTIFY_SECURE_PORT) {
      strcpy(record.host, "https://");
      offset = 8;
    } else {
      strcpy(record.host, "http://");
      offset = 7;
    }

    // Get the host value, including port number if it is not 80.
    size = kMaxHostLength - offset;
    suc = context->GetServerVariable(context, "HTTP_HOST",
                                     record.host + offset, &size);
    if (!suc) {
      // Use local address instead if host is not present.
      suc = context->GetServerVariable(context, "LOCAL_ADDR",
                                       record.host + offset, &size);
    }
    if (!suc) return SF_STATUS_REQ_NEXT_NOTIFICATION;

    // Content-Length, Last-Modified, and last-write time of file on disk
    // are only required for a successful status code.
    if (record.statuscode == 200) {      
      size = kBufferSize;
      if (suc && size > 0) {
        DLog(EVENT_IMPORTANT, "Some WWW-Authenticate url is ignored.");
        return SF_STATUS_REQ_NEXT_NOTIFICATION;
      }

      size = kBufferSize;
      if (suc && size > 0) {
        DLog(EVENT_IMPORTANT, "Some Proxy-Authenticate url is ignored.");
        return SF_STATUS_REQ_NEXT_NOTIFICATION;
      }

      // Get "Content-Length" header value.
      size = kBufferSize;
      suc = sendresponse->GetHeader(context, "Content-Length:", buffer, &size);
      record.contentHashCode = suc ? atoi(buffer) : -1;

      // Get "Last-Modified" header value.
      size = kBufferSize;
      suc = sendresponse->GetHeader(context, "Last-Modified:", buffer, &size);
      if (!suc || !BaseFilter::ParseTime(buffer, &record.last_modified)) {
        record.last_modified = -1;
      }

      // Set last_filewrite field of URL record, which is only available for
      // static web pages.
      // If the disk file size is the same with Content-Length header, the web
      // page is treated as static. Otherwise, BaseFilter::TreatAsStatic will
      // be used to determine whether the web page is static.
      //
      // Note: SCRIPT_TRANSLATED is only available under IIS 6.0+
      // SF_NOTIFY_URL_MAP could be used for IIS 4.x, 5.x, if necessary.
      size = kBufferSize;
      suc = context->GetServerVariable(context, "SCRIPT_TRANSLATED", buffer, &size);
      if (suc) {
        const char* filepath = buffer + 4;  // skip first four chars: "\\?\"
        FileAttribute file_attr;
        if (FileUtil::GetFileAttribute(filepath, &file_attr)) {
          record.last_filewrite = file_attr.last_modified;

          if (file_attr.size != record.contentHashCode) {
            if (!BaseFilter::TreatAsStatic(filepath)) {
              // A dynamic web page.
              record.last_filewrite = -1;
            }
          }
        } else {
          // Failed to get file attributes.
          record.last_filewrite = -1;
        }
      } else {
        // No corresponding file on disk.
        record.last_filewrite = -1;
      }
    }

    // Get http url, which is encoded already.
    size = kMaxUrlLength;
    suc = context->GetServerVariable(context, "HTTP_URL", record.url, &size);
    if (!suc || record.url[0] != '/') return SF_STATUS_REQ_NEXT_NOTIFICATION;

    Util::Log(EVENT_NORMAL,
            "Record generated:[url:%s|host:%s|siteid:%s|content:%lld|"
            "lastmod:%ld|lastwrite:%ld|status:%d]",
            record.url, record.host, record.siteid, record.contentHashCode,
            record.last_modified, record.last_filewrite, record.statuscode);

    BaseFilter::Send(&record, siteindex);
  }

  return SF_STATUS_REQ_NEXT_NOTIFICATION;
}

