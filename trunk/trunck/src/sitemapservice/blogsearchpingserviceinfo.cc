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


#include "sitemapservice/blogsearchpingserviceinfo.h"

BlogSearchPingServiceInfo::BlogSearchPingServiceInfo() {
  Reset();
}

void BlogSearchPingServiceInfo::Reset() {
  success_ = false;
  last_ping_ = -1;
  last_url_ = "N/A";
}

bool BlogSearchPingServiceInfo::Save(TiXmlElement* element) {
  SaveBoolAttribute(element, "success", success_);
  SaveTimeAttribute(element, "last_ping", last_ping_);
  SaveAttribute(element, "last_url", last_url_);

  return true;  // always success.
}

