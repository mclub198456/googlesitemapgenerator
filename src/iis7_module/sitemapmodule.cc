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


#include "iis7_module/sitemapmodule.h"

#include "common/util.h"
#include "common/logger.h"

SitemapModule::SitemapModule(BaseFilter* basefilter) {
  basefilter_ = basefilter;
}


REQUEST_NOTIFICATION_STATUS SitemapModule::OnLogRequest(
    IN IHttpContext* httpcontext,
    IN IHttpEventProvider* provider) {

  IHttpRequest* request = httpcontext->GetRequest();
  IHttpResponse* response = httpcontext->GetResponse();
  if (request == NULL || response == NULL) {
    return RQ_NOTIFICATION_CONTINUE;
  }

  UrlRecord record;

  // Check http method.
  if (request->GetHttpMethod()[0] != 'G') {
    return RQ_NOTIFICATION_CONTINUE;
  }

  // Get http status code.
  USHORT statuscode = 0;
  response->GetStatus(&statuscode);
  if (!basefilter_->CheckStatusCode(statuscode)) {
    return RQ_NOTIFICATION_CONTINUE;
  }
  record.statuscode = statuscode;

  // Ignore URLs requires authentication.
  if (statuscode == 200) {
    PCSTR auth = request->GetHeader(HttpHeaderAuthorization);
    if (auth != NULL) {
      DLog(EVENT_IMPORTANT, "Some url is ignored for Authorization");
      return RQ_NOTIFICATION_CONTINUE;
    }

    auth = request->GetHeader(HttpHeaderProxyAuthorization);
    if (auth != NULL) {
      DLog(EVENT_IMPORTANT, "Some url is ignored for ProxyAuthorization");
      return RQ_NOTIFICATION_CONTINUE;
    }
  }

  // Get http scheme. (HTTPS is not supported)
  int offset = 0;
  HTTP_SSL_CLIENT_CERT_INFO* client_certinfo = NULL;
  BOOL clientcert_negotiation;
  HRESULT result = request->GetClientCertificate(&client_certinfo,
                                                 &clientcert_negotiation);
  if (result == HRESULT_FROM_WIN32(ERROR_NOT_FOUND)
    || client_certinfo == NULL) {
    strcpy(record.host, "http://");
    offset = 7;
  } else {
    DLog(EVENT_IMPORTANT, "SSL is ignored.");
    return RQ_NOTIFICATION_CONTINUE;
  }

  // Get host name or ip address.
  // "SERVER_NAME" is a CGI meta-data. See RFC-3875
  PCSTR value = NULL;
  DWORD valuelen;
  if (httpcontext->GetServerVariable("SERVER_NAME", &value, &valuelen) != S_OK
    || valuelen + offset + 1 >= kMaxHostLength) {
    return RQ_NOTIFICATION_CONTINUE;
  }
  strcpy(record.host + offset, value);
  strlwr(record.host);

  // Get site id value.
  if (httpcontext->GetSite() == NULL) {
    return RQ_NOTIFICATION_CONTINUE;
  }
  itoa(httpcontext->GetSite()->GetSiteId(), record.siteid, 10);

  bool site_enabled = basefilter_->MatchSite(record.siteid);
  if (!site_enabled) {
    return RQ_NOTIFICATION_CONTINUE;
  }

  // Content length, last modified time and last file change
  // are only required to a successful status code.
  if (record.statuscode == 200) {
    record.contentHashCode = GetContentLength(response->GetRawHttpResponse());

    // Get last-modified http header.
    PCSTR header = response->GetHeader(HttpHeaderLastModified);
    if (header == NULL || !BaseFilter::ParseTime(header, &record.last_modified)) {
      record.last_modified = -1;
    }

    // Get last write time from static file.
    IHttpFileInfo* fileinfo = httpcontext->GetFileInfo();
    if (fileinfo != NULL) {

      ULARGE_INTEGER filesize;
      fileinfo->GetSize(&filesize);
      if (filesize.QuadPart == record.contentHashCode ||
        basefilter_->TreatAsStatic(bstr_t(fileinfo->GetFilePath()))) {

        FILETIME filetime;
        fileinfo->GetLastModifiedTime(&filetime);

        LARGE_INTEGER largeint;
        largeint.HighPart = filetime.dwHighDateTime;
        largeint.LowPart = filetime.dwLowDateTime;
        record.last_filewrite = largeint.QuadPart / 10000000 - 11644473600LL;
      } else {
        record.last_filewrite = -1;
      }
    } else {
      record.last_filewrite = -1;
    }
  }

  // Get http url, which is encoded already.
  HTTP_REQUEST* raw_request = request->GetRawHttpRequest();
  if (raw_request->RawUrlLength >= kMaxUrlLength) {
    return RQ_NOTIFICATION_CONTINUE;
  }
  strcpy(record.url, raw_request->pRawUrl);

  Logger::Log(EVENT_NORMAL,
              "url:%s; host:%s; siteid:%s; content:%lld; "
              "lastmod:%lld; lastwrite:%lld; status:%d;",
              record.url, record.host, record.siteid, record.contentHashCode,
              record.last_modified, record.last_filewrite, record.statuscode);
  basefilter_->Send(&record);

  return RQ_NOTIFICATION_CONTINUE;
}


int64 SitemapModule::GetContentLength(HTTP_RESPONSE* response) {
  int64 length = 0;
  for (USHORT i = 0; i < response->EntityChunkCount; ++i) {
    PHTTP_DATA_CHUNK chunk = response->pEntityChunks + i;
    if (chunk->DataChunkType == HttpDataChunkFromFileHandle) {
      // If the value is HTTP_BYTE_RANGE_TO_EOF, we just simply ignore it.
      ULONGLONG quadpart = chunk->FromFileHandle.ByteRange.Length.QuadPart;
      if (quadpart != HTTP_BYTE_RANGE_TO_EOF) {
        length += quadpart;
      }
    } else if (chunk->DataChunkType == HttpDataChunkFromMemory) {
      length += chunk->FromMemory.BufferLength;
    } else {
      return -1;
    }
  }

  return length;
}
