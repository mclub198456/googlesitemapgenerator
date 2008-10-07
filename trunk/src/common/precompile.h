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

//
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#ifndef COMMON_PRECOMPILE_H__
#define COMMON_PRECOMPILE_H__

// Allow use of features specific to Windows XP or later.
#ifndef WINVER
#define WINVER 0x0501
#endif

// Allow use of features specific to Windows XP or later.
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

// Allow use of features specific to Windows 98 or later.
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif

#ifndef _WIN32_IE  // Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600
#endif

// Exclude rarely-used stuff from Windows headers.
#define WIN32_LEAN_AND_MEAN
// Windows Header Files:
#define INITGUID

#include <specstrings.h>  // workaround for pulse build system

#include <windows.h>
#include <objbase.h>
#include <initguid.h>
#include <iads.h>
#include <adshlp.h>
#include <iiisext.h>
#include <comutil.h>
#include <tchar.h>

#include <string>
#include <vector>
#include <time.h>

#include "third_party/zlib/zlib.h"
#include "third_party/tinyxml/tinyxml.h"

#endif  // COMMON_PRECOMPILE_H__
