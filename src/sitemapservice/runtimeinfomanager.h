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


// This class is used to manage all runtime information in this application.
// It holds an ApplicationInfo object, which is a tree of runtime-info.
// All other objects shouldn't create runtime-info instance. If any object need
// to acess runtime-info, it should traverse the tree to get its part.
// Because the whole application shares the same ApplicationInfo object, this
// class also provides a lock. Any object wants to access any parts of of the
// runtime-info tree, it should call Lock/Unlock explicitly. The lock shouldn't
// be held for a long time.
// Currently this class is not implemented as a singleton, and all its methods
// are declared as static.

#ifndef SITEMAPSERVICE_RUNTIMEINFOMANAGER_H__
#define SITEMAPSERVICE_RUNTIMEINFOMANAGER_H__

#include <string>
#include "common/criticalsection.h"
#include "common/sitesettings.h"
#include "sitemapservice/applicationinfo.h"


class RuntimeInfoManager {
 public:
  // Get the ApplicationInfo object.
  static ApplicationInfo* application_info() { return &application_info_; }

  // DO NOT call this method in production code.
  // This method is a workaround for unit tests.
  static void set_application_info(const ApplicationInfo& application_info) {
    application_info_ = application_info;
  }

  // Reset the application_info to the un-touched status.
  static void Reset();

  // Initialize the application_info_ structure.
  // Usually, this field should only be called when starting up.
  static bool Initialize(SiteSettings settings);

  // Get XML string representation of runtime info.
  static bool GetRuntimeInfoString(std::string* xmlstr);

  // Save application_info_ to an XML document.
  // NULL is return if error occurs.
  // Invoker is responsible to delete the pointer after use.
  static TiXmlDocument* Save();

  static bool Lock(bool block);
  static void Unlock();

 private:
  // Prevent instantiation.
  RuntimeInfoManager() {}

  static ApplicationInfo application_info_;

  // Critical section used in Lock/Unlock methods.
  static CriticalSection critial_section_;
};

#endif  // SITEMAPSERVICE_RUNTIMEINFOMANAGER_H__
