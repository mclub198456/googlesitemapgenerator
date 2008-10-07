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


// This file defines the UrlFilter interface. A UrlFilter can be used to
// determine whether a given URL is accepted or not.
// Besides the interface, this file also provides two composite filters,
// AndFilter and OrFilter. They provides logic operations on filters.

#ifndef SITEMAPFILTER_URLFITLER_H__
#define SITEMAPFILTER_URLFITLER_H__

#include <list>
#include <string>

// Interface for url filter
class UrlFilter
{
public:
	// Determine whether to accept a url or not.
	// "url" the url to be analysed
	// "urlLen" the length of the url
	virtual bool Accept(const char* url,int urlLen) = 0; 
	virtual ~UrlFilter() {}
};

// A url is accepted only when it's accepted by all the filter.
class AndFilter : public UrlFilter {
 public:
	virtual ~AndFilter();
	void AddFilter(UrlFilter* filter);
	bool Accept(const char* url,int urlLen);

 private:
  typedef std::list<UrlFilter*>::const_iterator FILTER_CIT;
	typedef std::list<UrlFilter*>::iterator FILTER_IT;

	std::list<UrlFilter*> filters;
};

// A url is accepted once when it's accepted by certain filter.
class OrFilter : public UrlFilter {
 public:
	virtual ~OrFilter();
	void AddFilter(UrlFilter* filter);
	bool Accept(const char* url,int urlLen);

 private:
	typedef std::list<UrlFilter*>::const_iterator FILTER_CIT;
	typedef std::list<UrlFilter*>::iterator FILTER_IT;
  
  std::list<UrlFilter*> filters;
};

#endif  // SITEMAPFILTER_URLFITLER_H__

