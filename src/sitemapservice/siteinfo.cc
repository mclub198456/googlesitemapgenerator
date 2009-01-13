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


#include "sitemapservice/siteinfo.h"

SiteInfo::SiteInfo() {
  Reset();
}

void SiteInfo::Reset() {
  url_in_database_ = 0;
  url_in_tempfile_ = 0;
  url_in_memory_ = 0;

  host_name_ = "Undetermined";
  memory_used_ = 0;
  disk_used_ = 0;

  web_sitemapservice_info_.Reset();
  news_sitemapservice_info_.Reset();
  mobile_sitemapservice_info_.Reset();
  video_sitemapservice_info_.Reset();
  codesearch_sitemapservice_info_.Reset();
  blogsearch_pingservice_info_.Reset();

  logparser_info_.Reset();
  filescanner_info_.Reset();
  webserverfilter_info_.Reset();
}

bool SiteInfo::Save(TiXmlElement* element) {
  // Save simple attributes.
  SaveAttribute(element, "site_id", site_id_);
  SaveAttribute(element, "url_in_database", url_in_database_);
  SaveAttribute(element, "url_in_tempfile", url_in_tempfile_);
  SaveAttribute(element, "url_in_memory", url_in_memory_);
  SaveAttribute(element, "host_name", host_name_);
  SaveAttribute(element, "memory_used", memory_used_);
  SaveAttribute(element, "disk_used", disk_used_);

  // Construct the xml tree to save child elements.
  TiXmlElement* logparser_xml = new TiXmlElement("LogParserInfo");  
  element->LinkEndChild(logparser_xml);

  TiXmlElement* filescanner_xml = new TiXmlElement("FileScannerInfo");
  element->LinkEndChild(filescanner_xml);

  TiXmlElement* webserverfilter_xml = new TiXmlElement("WebServerFilterInfo");
  element->LinkEndChild(webserverfilter_xml);

  TiXmlElement* web_sitemapservice_xml =
    new TiXmlElement("WebSitemapServiceInfo");
  element->LinkEndChild(web_sitemapservice_xml);

  TiXmlElement* news_sitemapservice_xml =
    new TiXmlElement("NewsSitemapServiceInfo");
  element->LinkEndChild(news_sitemapservice_xml);

  TiXmlElement* video_sitemapservice_xml =
    new TiXmlElement("VideoSitemapServiceInfo");
  element->LinkEndChild(video_sitemapservice_xml);
  
  TiXmlElement* mobile_sitemapservice_xml =
    new TiXmlElement("MobileSitemapServiceInfo");
  element->LinkEndChild(mobile_sitemapservice_xml);

  TiXmlElement* codesearch_sitemapservice_xml =
    new TiXmlElement("CodeSearchSitemapServiceInfo");
  element->LinkEndChild(codesearch_sitemapservice_xml);

  TiXmlElement* blogsearch_pingservice_xml = 
    new TiXmlElement("BlogSearchPingServiceInfo");
  element->LinkEndChild(blogsearch_pingservice_xml);

  // Save the child elements.
  return logparser_info_.Save(logparser_xml)
  && filescanner_info_.Save(filescanner_xml)
  && webserverfilter_info_.Save(webserverfilter_xml)
  && web_sitemapservice_info_.Save(web_sitemapservice_xml)
  && news_sitemapservice_info_.Save(news_sitemapservice_xml)
  && video_sitemapservice_info_.Save(video_sitemapservice_xml)
  && mobile_sitemapservice_info_.Save(mobile_sitemapservice_xml)
  && codesearch_sitemapservice_info_.Save(codesearch_sitemapservice_xml)
  && blogsearch_pingservice_info_.Save(blogsearch_pingservice_xml);
}
