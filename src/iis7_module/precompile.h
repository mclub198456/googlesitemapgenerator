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

//
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#ifndef SITEMAPMODULE_STDAFX_H__
#define SITEMAPMODULE_STDAFX_H__

#ifdef WINVER
#if WINVER < 0x0600
#error WINVER is too small to build IIS module.
#endif
#else
#define WINVER 0x0600 // minimum value for LONGHORN
#endif


#ifdef _WIN32_WINNT
#if _WIN32_WINNT < 0x0600
#error _WIN32_WINNT is too small to build IIS module.
#endif
#else
#define _WIN32_WINNT 0x0600 // minimum value for LONGHORN
#endif

// Allow use of features specific to Windows 98 or later.
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif

// allow use IE 7.0 or later
#ifndef _WIN32_IE
#define _WIN32_IE 0x0700
#endif

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN	

// Windows Header Files:
#define INITGUID

#include <windows.h>
#include <httpfilt.h>
#include <objbase.h>
#include <initguid.h>
#include <iads.h>
#include <adshlp.h>
#include <iiisext.h>
#include <comutil.h>

#include <string>

#endif // SITEMAPMODULE_STDAFX_H__
