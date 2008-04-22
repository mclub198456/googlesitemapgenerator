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


#include "common/iis6config.h"

#include <objbase.h>
#include <initguid.h>
#include <iads.h>
#include <adshlp.h>
#include <comutil.h>
#include <string>

#include "common/url.h"
#include "common/util.h"

bool Iis6Config::Load() {
  HRESULT         hr = S_OK;
  IADsContainer*  web_service = NULL;
  IUnknown*       unknown  = NULL;
  IEnumVARIANT*   enumerator  = NULL;
  ULONG           fetch_number;

  Util::Log(EVENT_CRITICAL, "Loading setting from IIS");

  site_ids_.clear();
  names_.clear();
  host_urls_.clear();
  physical_paths_.clear();

  do {
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR, "CoInitialize failed. Error 0x%0x", hr);
      break;
    }

    hr = ADsGetObject(L"IIS://localhost/w3svc",
                      IID_IADsContainer,
                      reinterpret_cast<void **>(&web_service));
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR, "ADsGetObject() failed. Error 0x%0x", hr);
      break;
    }

    hr = web_service->get__NewEnum(&unknown);
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR, "get__NewEnum() failed. Error 0x%0x", hr);
      break;
    }

    hr = unknown->QueryInterface(IID_IEnumVARIANT,
                                 reinterpret_cast<void **>(&enumerator));
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR,
                "QueryInterface(IID_IEnumVARIANT) failed. Error 0x%0x",
                hr);
      break;
    }

    // Enumerate all web sites
    do {
      IDispatch*      dispatch = NULL;
      IADs*           web_server = NULL;
      BSTR            server_class = NULL;
      VARIANT         variant;
      VARIANT         server_bindings;

      VariantInit(&variant);
      VariantInit(&server_bindings);

      hr = enumerator->Next(1, &variant, &fetch_number);
      if (FAILED(hr) || hr == S_FALSE || fetch_number < 1)
        break;

      do {
        if (variant.vt != VT_DISPATCH)
          continue;

        dispatch = V_DISPATCH(&variant);

        hr = dispatch->QueryInterface(IID_IADs,
                                      reinterpret_cast<void **>(&web_server));
        if (FAILED(hr)) {
          Util::Log(EVENT_ERROR,
                    "QueryInterface(IID_IADs) failed. Error 0x%0x",
                    hr);
          break;
        }

        hr = web_server->get_Class(&server_class);
        if (FAILED(hr)) {
          Util::Log(EVENT_ERROR, "get_Class() failed. Error 0x%0x", hr);
          break;
        }

        // Skip none IIsWebServer object.
        if (_bstr_t(server_class) != _bstr_t("IIsWebServer"))
          break;

        hr = LoadSiteId(web_server);
        if (FAILED(hr)) {
          Util::Log(EVENT_ERROR, "Failed to load site id (0x%0x)", hr);
          break;
        }

        hr = LoadName(web_server);
        if (FAILED(hr)) {
          Util::Log(EVENT_ERROR, "Failed to load site name (0x%0x)", hr);
          break;
        }

        hr = LoadServerBindings(web_server);
        if (FAILED(hr)) {
          Util::Log(EVENT_ERROR, "Failed to load server bindings (0x%0x)", hr);
          break;
        }

        hr = LoadPhysicalPath(web_server);
        if (FAILED(hr)) {
          Util::Log(EVENT_ERROR, "Failed to load physical path (0x%0x)", hr);
          break;
        }

        hr = LoadLogPath(web_server);
        if (FAILED(hr)) {
          Util::Log(EVENT_ERROR, "Failed to load log path (0x%0x)", hr);
          break;
        }

      } while (false);

      if (server_class) SysFreeString(server_class);
      if (dispatch) dispatch->Release();
      if (web_server) web_server->Release();

    } while (true);

  } while (false);

  if (enumerator) enumerator->Release();
  if (unknown) unknown->Release();
  if (web_service) web_service->Release();

  CoUninitialize();

  return SUCCEEDED(hr);
}

HRESULT Iis6Config::LoadSiteId(IADs *web_server) {
  HRESULT         hr = S_OK;
  BSTR            site_id = NULL;

  hr = web_server->get_Name(&site_id);
  if (FAILED(hr)) {
    Util::Log(EVENT_ERROR, "get_Name() failed. Error 0x%0x", hr);
  } else {
    site_ids_.push_back(static_cast<char *>(bstr_t(site_id)));
  }

  if (site_id)
    SysFreeString(site_id);

  return hr;
}

HRESULT Iis6Config::LoadName(IADs *web_server) {
  HRESULT         hr = S_OK;
  VARIANT         server_comment;

  VariantInit(&server_comment);

  hr = web_server->Get(_bstr_t("ServerComment"), &server_comment);
  if (FAILED(hr)) {
    Util::Log(EVENT_ERROR, "Get(ServerComment) failed. Error 0x%0x", hr);
  } else {
    names_.push_back(static_cast<char *>(bstr_t(server_comment.bstrVal)));
  }

  VariantClear(&server_comment);

  return hr;
}

HRESULT Iis6Config::LoadLogPath(IADs *web_server) {
  HRESULT         hr = S_OK;
  VARIANT         logfile_directory;

  VariantInit(&logfile_directory);

  hr = web_server->Get(_bstr_t("LogFileDirectory"), &logfile_directory);
  if (FAILED(hr)) {
    Util::Log(EVENT_ERROR, "Get(LogFileDirectory) failed. Error 0x%0x", hr);
  } else {
    log_paths_.push_back(static_cast<char *>(bstr_t(logfile_directory.bstrVal)));
    log_paths_.back().append("\\W3SVC").append(site_ids_.back());
  }

  VariantClear(&logfile_directory);

  return hr;
}

HRESULT Iis6Config::LoadServerBindings(IADs *web_server) {
  HRESULT         hr = S_OK;
  VARIANT         server_bindings;

  VariantInit(&server_bindings);

  do {
    hr = web_server->Get(_bstr_t("ServerBindings"), &server_bindings);
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR, "Get(ServerBindings) failed. Error 0x%0x", hr);
      break;
    }

    if (server_bindings.vt != (VT_VARIANT|VT_ARRAY)
      || server_bindings.parray->cDims != 1 ) {
      hr = E_FAIL;
      Util::Log(EVENT_ERROR,
                "ServerBindings is not in desired format. Error 0x%0x",
                hr);
      break;
    }

    long            binding_index;
    long            binding_left_index;
    long            binding_right_index;
    std::string     host_url;

    hr = SafeArrayGetLBound(server_bindings.parray, 1, &binding_left_index);
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR, "SafeArrayGetLBound failed. Error 0x%0x", hr);
      break;
    }

    hr = SafeArrayGetUBound(server_bindings.parray, 1, &binding_right_index);
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR, "SafeArrayGetRBound failed. Error 0x%0x", hr);
      break;
    }

    for (binding_index = binding_left_index;
      binding_index <= binding_right_index;binding_index++) {
      VARIANT         server_binding;

      do {
        VariantInit(&server_binding);

        hr = SafeArrayGetElement(server_bindings.parray,
                                 &binding_index,
                                 &server_binding);
        if (FAILED(hr)) {
          Util::Log(EVENT_ERROR, "SafeArrayGetElement failed. Error 0x%0x", hr);
          break;
        }

        if (server_binding.vt != VT_BSTR) {
          hr = E_FAIL;
          Util::Log(EVENT_ERROR,
                    "ServerBinding is not in BSTR. Error 0x%0x",
                    hr);
          break;
        }

        // Parse the server_binding string.
        // The binding string is in "IP:PORT:HOST" format.
        std::string binding_string(
          static_cast<char *>(bstr_t(server_binding.bstrVal)));
        size_t position1 = binding_string.find(":");
        size_t position2 = binding_string.find(":", position1+1);
        if (position1 == std::string::npos || position2 == std::string::npos) {
          hr = E_FAIL;
          Util::Log(EVENT_ERROR,
                    "ServerBinding is not in desired format. Error 0x%0x", hr);
          break;
        }

        // Use the first valid binding_string as the host name.
        if (binding_string.substr(position2+1).size() > 0) {
          std::string host_url(binding_string.substr(position2+1));
          host_url.append(":");
          host_url.append(
            binding_string.substr(position1+1, position2-position1-1));
        }
      } while (false);

      VariantClear(&server_binding);

      if (host_url.size() != 0)
        break;
    }

    host_urls_.push_back(host_url);
  } while (false);

  VariantClear(&server_bindings);

  return hr;
}

HRESULT Iis6Config::LoadPhysicalPath(IADs *web_server) {
  HRESULT         hr = S_OK;
  IADsContainer*  server_container = NULL;
  IUnknown*       unknown  = NULL;
  IEnumVARIANT*   enumerator  = NULL;
  ULONG           fetch_number;

  do {
    hr = web_server->QueryInterface(IID_IADsContainer,
      reinterpret_cast<void **>(&server_container));
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR,
                "QueryInterface(IID_IADsContainer) failed. Error 0x%0x",
                hr);
      break;
    }

    hr = server_container->get__NewEnum(&unknown);
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR, "get__NewEnum() failed. Error 0x%0x", hr);
      break;
    }

    hr = unknown->QueryInterface(IID_IEnumVARIANT,
                                 reinterpret_cast<void **>(&enumerator));
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR,
                "QueryInterface(IID_IEnumVARIANT) failed. Error 0x%0x",
                hr);
      break;
    }

    // Enumerate all web sites
    do {
      IDispatch*      dispatch = NULL;
      IADs*           web_virtual_dir = NULL;
      BSTR            virtual_dir_class = NULL;
      VARIANT         variant;
      VARIANT         path;

      VariantInit(&variant);
      VariantInit(&path);

      hr = enumerator->Next(1, &variant, &fetch_number);
      if (FAILED(hr) || hr == S_FALSE || fetch_number < 1)
        break;

      do {
        if (variant.vt != VT_DISPATCH)
          continue;

        dispatch = V_DISPATCH(&variant);

        hr = dispatch->QueryInterface(IID_IADs,
          reinterpret_cast<void **>(&web_virtual_dir));
        if (FAILED(hr)) {
          Util::Log(EVENT_ERROR,
                    "QueryInterface(IID_IADs) failed. Error 0x%0x",
                    hr);
          break;
        }

        hr = web_virtual_dir->get_Class(&virtual_dir_class);
        if (FAILED(hr)) {
          Util::Log(EVENT_ERROR, "get_Class() failed. Error 0x%0x", hr);
          break;
        }

        // Skip none IIsWebVirtualDir object.
        if (_bstr_t(virtual_dir_class) != _bstr_t("IIsWebVirtualDir"))
          break;

        hr = web_virtual_dir->Get(_bstr_t("Path"), &path);
        if (FAILED(hr)) {
          Util::Log(EVENT_ERROR, "Get(Path) failed. Error 0x%0x", hr);
          break;
        }

        physical_paths_.push_back(static_cast<char *>(bstr_t(path.bstrVal)));

      } while (false);

      VariantClear(&path);

      if (virtual_dir_class) SysFreeString(virtual_dir_class);
      if (web_virtual_dir) web_virtual_dir->Release();
      if (dispatch) dispatch->Release();
    } while (true);

  } while (false);

  if (enumerator) enumerator->Release();
  if (unknown) unknown->Release();
  if (server_container) server_container->Release();

  return hr;
}

const wchar_t *kFilterName = L"GoogleSitemapGeneratorFilter";

bool Iis6Config::InstallFilter(const wchar_t* dll_file) {
  HRESULT        hr = S_OK;
  IADsContainer  *ads_container = NULL;
  IADs *         filters = NULL;
  IADs*          filter = NULL;
  VARIANT        filter_order;

  Util::Log(EVENT_CRITICAL, "Installing filter");

  do {
    VariantInit(&filter_order);

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR, "CoInitialize failed. Error 0x%0x", hr);
      break;
    }

    hr = ADsGetObject(L"IIS://localhost/w3svc/Filters",
                      IID_IADsContainer,
                      reinterpret_cast<void **>(&ads_container));
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR, "ADsGetObject() failed. Error 0x%0x", hr);
      break;
    }

    hr = ads_container->QueryInterface(IID_IADs,
                                       reinterpret_cast<void **>(&filters));
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR,
                "ads_container->QueryInterface(IID_IADs) failed. Error 0x%0x",
                hr);
      break;
    }

    filters->Get(_bstr_t("FilterLoadOrder"), &filter_order);
    if (filter_order.vt != VT_BSTR) {
      hr = E_FAIL;
      Util::Log(EVENT_ERROR, "filters->FilterLoadOrder() failed.");
      break;
    }

    std::wstring temp_string = filter_order.bstrVal;
    if (temp_string.find(kFilterName) == std::string::npos) {
      // Add filter to filter_order list.
      if (temp_string.length() != 0 &&
        temp_string[temp_string.length() - 1] != L',') {
        temp_string += L",";
      }
      temp_string += kFilterName;

      // Save setting
      filter_order.bstrVal = _bstr_t(temp_string.c_str()).Detach();

      hr = filters->Put(_bstr_t("FilterLoadOrder"), filter_order);
      if (FAILED(hr)) {
        Util::Log(EVENT_ERROR, "filters->Put() failed. Error 0x%0x", hr);
        break;
      }

      hr = filters->SetInfo();
      if (FAILED(hr)) {
        Util::Log(EVENT_ERROR, "filters->SetInfo() failed. Error 0x%0x", hr);
        break;
      }
    }

    // Add or update filter object
    hr = ads_container->GetObject(_bstr_t("IIsFilter"),
                                  _bstr_t(kFilterName),
                                  reinterpret_cast<IDispatch**>(&filter));
    if (FAILED(hr) || NULL == filter) {
      hr = ads_container->Create(_bstr_t("IIsFilter"),
                                 _bstr_t(kFilterName),
                                 reinterpret_cast<IDispatch**>(&filter));
      if (FAILED(hr)) {
        Util::Log(EVENT_ERROR, "Create filter failed. Error 0x%0x", hr);
        break;
      }
    }

    hr = filter->Put(_bstr_t("FilterPath"), _variant_t(dll_file));
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR, "filter->Put(FilterPath) failed. Error 0x%0x", hr);
      break;
    }

    hr = filter->Put(_bstr_t("FilterDescription"),
                     _variant_t("Google Sitemap Generator Filter"));
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR,
                "filter->Put(FilterDescription) failed. Error 0x%0x",
                hr);
      break;
    }

    hr = filter->Put(_bstr_t("FilterFlags"), _variant_t(0x00020000));
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR,
                "filter->Put(FilterFlags) failed. Error 0x%0x",
                hr);
      break;
    }

    hr = filter->SetInfo();
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR, "filter->SetInfo() failed. Error 0x%0x", hr);
      break;
    }
  } while (false);

  VariantClear(&filter_order);
  if (filter) filter->Release();
  if (filters) filters->Release();
  if (ads_container) ads_container->Release();

  CoUninitialize();

  if (FAILED(hr)) {
    return false;
  } else {
    Util::Log(EVENT_CRITICAL, "Filter installed successfully");
    return true;
  }
}

bool Iis6Config::UninstallFilter() {
  HRESULT        hr = S_OK;
  IADsContainer  *ads_container = NULL;
  IADs *         filters = NULL;
  IADs*          filter = NULL;
  VARIANT        filter_order;

  Util::Log(EVENT_CRITICAL, "Uninstalling Filter");

  do {
    VariantInit(&filter_order);

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR, "CoInitialize failed. Error 0x%0x", hr);
      break;
    }

    hr = ADsGetObject(L"IIS://localhost/w3svc/Filters",
                      IID_IADsContainer,
                      reinterpret_cast<void **>(&ads_container));
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR, "ADsGetObject() failed. Error 0x%0x", hr);
      break;
    }

    hr = ads_container->QueryInterface(IID_IADs,
                                       reinterpret_cast<void **>(&filters));
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR,
                "ads_container->QueryInterface(IID_IADs) failed. Error 0x%0x",
                hr);
      break;
    }

    filters->Get(_bstr_t("FilterLoadOrder"), &filter_order);
    if (filter_order.vt != VT_BSTR) {
      hr = E_FAIL;
      Util::Log(EVENT_ERROR, "filters->FilterLoadOrder() failed.");
      break;
    }

    std::wstring temp_string = filter_order.bstrVal;
    size_t index = temp_string.find(kFilterName);
    if (index != std::string::npos) {
      // Remove filter name
      temp_string.push_back(L',');
      temp_string.erase(index, wcslen(kFilterName) + 1);
      if (temp_string.length() > 0) {
        temp_string.erase(temp_string.length() - 1);
      }

      // Save setting
      filter_order.bstrVal = bstr_t(temp_string.c_str()).Detach();

      hr = filters->Put(_bstr_t("FilterLoadOrder"), filter_order);
      if (FAILED(hr)) {
        Util::Log(EVENT_ERROR, "filters->Put() failed. Error 0x%0x", hr);
        break;
      }

      hr = filters->SetInfo();
      if (FAILED(hr)) {
        Util::Log(EVENT_ERROR, "filters->SetInfo() failed. Error 0x%0x", hr);
        break;
      }
    }

    // Remove filter object
    hr = ads_container->GetObject(_bstr_t("IIsFilter"),
                                  _bstr_t(kFilterName),
                                  reinterpret_cast<IDispatch**>(&filter));
    if (FAILED(hr) || NULL == filter) {
      hr = S_OK;
      Util::Log(EVENT_CRITICAL, "Filter doesn't exist.");
      break;
    }

    hr = ads_container->Delete(_bstr_t("IIsFilter"), _bstr_t(kFilterName));
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR, "Delete filter failed. Error 0x%0x", hr);
      break;
    }
  } while (false);

  VariantClear(&filter_order);
  if (filter) filter->Release();
  if (filters) filters->Release();
  if (ads_container) ads_container->Release();

  CoUninitialize();

  if (FAILED(hr)) {
    return false;
  } else {
    Util::Log(EVENT_CRITICAL, "Filter uninstalled successfully");
    return true;
  }
}

bool Iis6Config::IsSupported() {
  HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
  if (FAILED(hr)) {
    Util::Log(EVENT_ERROR, "Failed to check IIS 6 compatibility. 0x%0x", hr);
    return false;
  }

  // Try with IIS 6.0...
  IADsContainer*  web_service = NULL;
  hr = ADsGetObject(L"IIS://localhost/w3svc",
                    IID_IADsContainer,
                    reinterpret_cast<void **>(&web_service));
  if (SUCCEEDED(hr)) {
    web_service->Release();
  }

  CoUninitialize();

  return SUCCEEDED(hr);
}

IisConfig::AppMode Iis6Config::GetAppMode() {
  SYSTEM_INFO sys_info;
  GetNativeSystemInfo(&sys_info);
  if (sys_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
    return MODE_X86;
  } else if (sys_info.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_AMD64) {
    return MODE_UNKNOWN;
  }

  // For IIS running on AMD64, it may support two modes.
  // It is dtermined by IIS://localhost/w3svc/AppPools/Enable32bitAppOnWin64.
  HRESULT        hr = S_OK;
  IADsContainer  *ads_container = NULL;
  IADs *         pools = NULL;
  VARIANT        mode_attr;
  bool           mode_value;

  VariantInit(&mode_attr);
  do {
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR, "CoInitialize failed. Error 0x%0x", hr);
      break;
    }

    hr = ADsGetObject(L"IIS://localhost/w3svc/AppPools",
                      IID_IADsContainer,
                      reinterpret_cast<void **>(&ads_container));
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR, "ADsGetObject() failed. Error 0x%0x", hr);
      break;
    }

    hr = ads_container->QueryInterface(IID_IADs,
                                       reinterpret_cast<void **>(&pools));
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR,
                "ads_container->QueryInterface(IID_IADs) failed. Error 0x%0x",
                hr);
      break;
    }

    hr = pools->Get(_bstr_t("Enable32bitAppOnWin64"), &mode_attr);
    if (FAILED(hr)) {
      Util::Log(EVENT_ERROR, "Failed to get Enable32bitAppOnWin64 value. %0x",
                hr);
    }

    if (mode_attr.vt != VT_BOOL) {
      hr = E_FAIL;
      Util::Log(EVENT_ERROR, "pools->Enable32bitAppOnWin64() is not BOOL.");
      break;
    }

    mode_value = (mode_attr.boolVal != 0);
  } while (false);

  // Release resources.
  VariantClear(&mode_attr);
  if (pools != NULL) pools->Release();
  if (ads_container != NULL) ads_container->Release();
  CoUninitialize();

  if (FAILED(hr)) {
    return MODE_FAIL;
  } else {
    return mode_value ? MODE_X86 : MODE_X64;
  }
}
