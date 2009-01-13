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


// Factory class is used to create SitemapModule. This factory class is
// required by IIS7 module API.

#ifndef IIS7_MODULE_SITEMAPMODULEFACTORY_H__
#define IIS7_MODULE_SITEMAPMODULEFACTORY_H__

#include <httpserv.h>

class BaseFilter;

class SitemapModuleFactory : public IHttpModuleFactory {
 public:
  SitemapModuleFactory();
  virtual ~SitemapModuleFactory();

  // Create sitemap module.
  HRESULT GetHttpModule(OUT CHttpModule ** pp_module,
    IN IModuleAllocator * allocator);

  // Initlialize this factory.
  bool Initialize();

  // Terminate this factory.
  void Terminate();

 private:
  // Base filter passed to constructor of SitemapModule.
  // It is created in Initilize method and released in Terminate method.
  BaseFilter* basefilter_;
};

#endif  // IIS7_MODULE_SITEMAPMODULEFACTORY_H__
