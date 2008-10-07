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


#include "common/iis7config.h"

#include <ahadmin.h>
#include "common/logger.h"

BSTR Iis7Config::kModuleName = L"GoogleSitemapGeneratorModule";
BSTR Iis7Config::kSiteName = L"Google Sitemap Generator Admin Console";

#define CHECK_HRESULT(expression, msg) \
{ \
HRESULT _hr = (expression); \
if (FAILED(_hr)) { \
  Logger::Log(EVENT_ERROR, msg " (0x%0x)", _hr); \
  break; \
} \
}
#define CHECK_BOOL(expression, msg) \
  if (!expression) { \
    Logger::Log(EVENT_ERROR, msg); \
    break; \
  }

bool Iis7Config::Load() {
  IAppHostAdminManager*   admin_manager = NULL;
  IAppHostElement*        parent = NULL;
  IAppHostElementCollection* collection = NULL;
  IAppHostElement*        element = NULL;

  BSTR  section_name = SysAllocString(L"system.applicationHost/sites");
  BSTR bstr_config_path = SysAllocString(L"MACHINE/WEBROOT/APPHOST");

  HRESULT hresult = S_OK;
  bool result = false;
  do {
    // Initialize com
    hresult = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Failed to intialize COM.");
      break;
    }

    // Create an admin manager
    hresult = CoCreateInstance(__uuidof(AppHostAdminManager), NULL, 
              CLSCTX_INPROC_SERVER,
              __uuidof(IAppHostAdminManager),
              reinterpret_cast<LPVOID*>(&admin_manager));
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Failed to create AppHostAdminManager.");
      break;
    }

    // Get <sites> section
    hresult = admin_manager->GetAdminSection(section_name, bstr_config_path, &parent);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Unable to access sites configuration.");
      break;
    } else if (&parent == NULL) {
      Logger::Log(EVENT_ERROR, "Unable to get sites configuration."); 
      break;
    }

    // Get all the child elements of <sites>
    hresult = parent->get_Collection(&collection);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Unable to access <sites> child collection.");
      break;
    } else if (&collection == NULL) {
      Logger::Log(EVENT_ERROR, "Unable to get <sites> child collection."); 
      break;
    }

    // Iterate all the children of <sites>
    DWORD element_count = 0;
    hresult = collection->get_Count(&element_count);
    for(USHORT i = 0; i < element_count; ++i) {
      VARIANT item_index;
      item_index.vt = VT_I2;
      item_index.iVal = i;

      // Get the child element
      hresult = collection->get_Item(item_index, &element);
      if (FAILED(hresult) || &element == NULL) {
        Logger::Log(EVENT_ERROR, "Failed to iterate child (%d) of sites.", i);
        continue;
      }

      if (!LoadSite(element)) {
        Logger::Log(EVENT_ERROR, "Failed to load site from site (%d).", i);
        continue;
      }

      element->Release();
      element = NULL;
    }

    result = true;
  } while (false);

  // Exiting / Unwinding
  if (element != NULL) element->Release();
  if (collection != NULL) collection->Release();
  if (parent != NULL) parent->Release();
  if (admin_manager != NULL) admin_manager->Release();

  SysFreeString(section_name);
  SysFreeString(bstr_config_path);
  CoUninitialize();

  return result;
}

bool Iis7Config::InstallFilter(const wchar_t* dll_file) {
  IAppHostWritableAdminManager*   admin_manager = NULL;
  BSTR bstr_config_path = SysAllocString(L"MACHINE/WEBROOT/APPHOST");

  // Build the full path of sitemap module DLL.
  BSTR module_wpath = SysAllocString(dll_file);

  HRESULT hresult = S_OK;
  bool result = false;
  do {
    // Initialize com
    hresult = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Failed to intialize COM.");
      break;
    }

    // Create a writable admin manager
    hresult = CoCreateInstance(__uuidof(AppHostWritableAdminManager), NULL, 
              CLSCTX_INPROC_SERVER,
              __uuidof(IAppHostWritableAdminManager),
              reinterpret_cast<LPVOID*>(&admin_manager));
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Failed to create AppHostWritableAdminManager.");
      break;
    }

    // set commit path
    hresult = admin_manager->put_CommitPath(L"MACHINE/WEBROOT/APPHOST");
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Failed to put commit path.");
      break;
    }

    // Add sitemap entry to <system.webServer/globalModules> section
    if (!AddToGlobalModules(admin_manager, module_wpath)) {
      Logger::Log(EVENT_ERROR, "Failed to add entry to globalModules.");
      break;
    }

    // Add sitemap entry to <system.webServer/modules> section
    if (!AddToModules(admin_manager)) {
      Logger::Log(EVENT_ERROR, "Failed to add entry to modules.");
      break;
    }

    // Save changes.
    hresult = admin_manager->CommitChanges();
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Failed to save changes to install module.");
      break;
    }

    result = true;
  } while (false);

  // Exiting / Unwinding
  if (admin_manager != NULL) admin_manager->Release();

  SysFreeString(module_wpath);
  SysFreeString(bstr_config_path);

  CoUninitialize();

  return result;
}

bool Iis7Config::UninstallFilter() {
  IAppHostWritableAdminManager*   admin_manager = NULL;
  BSTR bstr_config_path = SysAllocString(L"MACHINE/WEBROOT/APPHOST");

  HRESULT hresult = S_OK;
  bool result = false;
  do {
    // Initialize com
    hresult = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Failed to intialize COM.");
      break;
    }

    // Create a writable admin manager
    hresult = CoCreateInstance(__uuidof(AppHostWritableAdminManager), NULL, 
              CLSCTX_INPROC_SERVER,
              __uuidof(IAppHostWritableAdminManager),
              reinterpret_cast<LPVOID*>(&admin_manager));
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Failed to create AppHostWritableAdminManager.");
      break;
    }

    // set commit path
    hresult = admin_manager->put_CommitPath(bstr_config_path);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Failed to put commit path.");
      break;
    }

    // Remove sitemap entry from <system.webServer/globalModules> section.
    if (!RemoveFromCollection(admin_manager, L"system.webServer/globalModules", L"name", kModuleName)) {
      Logger::Log(EVENT_ERROR, "Failed to remove entry from globalModules.");
      break;
    }

    // Remove sitemap entry from <system.webServer/modules> section.
    if (!RemoveFromCollection(admin_manager, L"system.webServer/modules", L"name", kModuleName)) {
      Logger::Log(EVENT_ERROR, "Failed to remove entry from modules.");
      break;
    }

    // Save changes.
    hresult = admin_manager->CommitChanges();
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Failed to save changes to install module.");
      break;
    }

    result = true;
  } while (false);

  // Exiting / Unwinding
  if (admin_manager != NULL) admin_manager->Release();
  SysFreeString(bstr_config_path);

  CoUninitialize();

  return result;
}

// Add an entry to <system.webServer/globalModules> section, like:
// <add name="GoogleSitemapGeneratorModule" image="${image}" />
bool Iis7Config::AddToGlobalModules(IAppHostWritableAdminManager* manager,
                                         BSTR image) {
  IAppHostElement*        parent = NULL;
  IAppHostElementCollection* collection = NULL;
  IAppHostElement*        element = NULL;

  BSTR bstr_section_name = SysAllocString(L"system.webServer/globalModules");
  BSTR bstr_config_path = SysAllocString(L"MACHINE/WEBROOT/APPHOST");
  BSTR bstr_element_name = SysAllocString(L"add");

  HRESULT hresult = S_OK;
  bool result = false;
  do {
    // Get <system.webServer/globalModules> section
    hresult = manager->GetAdminSection(bstr_section_name,
                                       bstr_config_path, &parent);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Unable to access globalModules configuration.");
      break;
    } else if (&parent == NULL) {
      Logger::Log(EVENT_ERROR, "Unable to get globalModules configuration."); 
      break;
    }

    // Get collection of <system.webServer/globalModules>
    hresult = parent->get_Collection(&collection);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Unable to access globalModules child collection.");
      break;
    } else if (&collection == NULL) {
      Logger::Log(EVENT_ERROR, "Unable to get globalModules child collection."); 
      break;
    }

    // create a global modules child element, like:
    // <add name="GoogleSitemapGeneratorModule", image="sitemap_module.dll" />

    // First to detect old modules.
    if (!GetFromCollection(collection, L"name", kModuleName, &element)) {
      Logger::Log(EVENT_ERROR, "Failed to try detect old modules.");
      break;
    }

    // No old sitemap module, create/add one.
    if (element == NULL) {
      hresult = collection->CreateNewElement(bstr_element_name, &element);
      if (FAILED(hresult)) {
        Logger::Log(EVENT_ERROR, "Failed to create globalModules/add element.");
        break;
      }

      if (!SetProperty(element, L"name", kModuleName)) {
        Logger::Log(EVENT_ERROR, "Failed to set name property.");
        break;
      }
      // Add the new element to collection
      hresult = collection->AddElement(element);
      if (FAILED(hresult)) {
        Logger::Log(EVENT_ERROR, "Failed to add globalModule/add element.");
        break;
      }
    }

    if (!SetProperty(element, L"image", image)) {
      Logger::Log(EVENT_ERROR, "Failed to set image property.");
      break;
    }

    result = true;
  } while (false);

  // Exiting / Unwinding
  if (element != NULL) element->Release();
  if (collection != NULL) collection->Release();
  if (parent != NULL) parent->Release();

  SysFreeString(bstr_section_name);
  SysFreeString(bstr_config_path);
  SysFreeString(bstr_element_name);

  return result;
}


// Add an entry to <system.webServer/modules> section, like:
// <add name="GoogleSitemapGeneratorModule" />
bool Iis7Config::AddToModules(IAppHostWritableAdminManager* manager) {
  IAppHostElement*        parent = NULL;
  IAppHostElementCollection* collection = NULL;
  IAppHostElement*        element = NULL;

  BSTR bstr_section_name = SysAllocString(L"system.webServer/modules");
  BSTR bstr_config_path = SysAllocString(L"MACHINE/WEBROOT/APPHOST");
  BSTR bstr_element_name = SysAllocString(L"add");

  HRESULT hresult = S_OK;
  bool result = false;
  do {
    // Get <system.webServer/modules> section
    hresult = manager->GetAdminSection(bstr_section_name,
                                       bstr_config_path, &parent);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Unable to access modules configuration.");
      break;
    } else if (&parent == NULL) {
      Logger::Log(EVENT_ERROR, "Unable to get modules configuration."); 
      break;
    }

    // Get collection of <system.webServer/modules>
    hresult = parent->get_Collection(&collection);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Unable to access modules child collection.");
      break;
    } else if (&collection == NULL) {
      Logger::Log(EVENT_ERROR, "Unable to get modules child collection."); 
      break;
    }

    // create a modules child element, like:
    // <add name="GoogleSitemapGeneratorModule" />

    // Try to detect old sitemap module.
    if (!GetFromCollection(collection, L"name", kModuleName, &element)) {
      Logger::Log(EVENT_ERROR, "Failed to try detect old modules.");
      break;
    }

    // No old sitemap module exists.
    if (element == NULL) {
      hresult = collection->CreateNewElement(bstr_element_name, &element);
      if (FAILED(hresult)) {
        Logger::Log(EVENT_ERROR, "Failed to create modules/add element.");
        break;
      }

      if (!SetProperty(element, L"name", kModuleName)) {
        Logger::Log(EVENT_ERROR, "Failed to set name property.");
        break;
      }

      // Add the new element to collection
      hresult = collection->AddElement(element);
      if (FAILED(hresult)) {
        Logger::Log(EVENT_ERROR, "Failed to add modules/add element.");
        break;
      }
    }

    result = true;
  } while (false);

  // Exiting / Unwinding
  if (element != NULL) element->Release();
  if (collection != NULL) collection->Release();
  if (parent != NULL) parent->Release();

  SysFreeString(bstr_section_name);
  SysFreeString(bstr_config_path);
  SysFreeString(bstr_element_name);

  return result;
}

bool Iis7Config::RemoveFromCollection(IAppHostWritableAdminManager* manager,
                                     BSTR section, BSTR property_name, BSTR property_value) {
  IAppHostElement*        parent = NULL;
  IAppHostElementCollection* collection = NULL;
  IAppHostElement*        element = NULL;
  IAppHostProperty*       name_property = NULL;

  BSTR bstr_section_name = SysAllocString(section);
  BSTR bstr_config_path = SysAllocString(L"MACHINE/WEBROOT/APPHOST");

  HRESULT hresult = S_OK;
  bool result = false;
  do {
    // Get section contains SitemapModule entry
    hresult = manager->GetAdminSection(bstr_section_name,
                                       bstr_config_path, &parent);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Unable to access section to remove module.");
      break;
    } else if (&parent == NULL) {
      Logger::Log(EVENT_ERROR, "Unable to get section to remove module."); 
      break;
    }

    // Get child collection of the section
    hresult = parent->get_Collection(&collection);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Unable to access collection to remove module.");
      break;
    } else if (&collection == NULL) {
      Logger::Log(EVENT_ERROR, "Unable to get collection to remove module."); 
      break;
    }

    // Get number of children.
    DWORD count;
    hresult = collection->get_Count(&count);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Unable to get the count of collection.");
      break;
    }

    short sitemap_index;
    if (!GetFromCollection(collection, property_name, property_value, &sitemap_index)) {
      Logger::Log(EVENT_ERROR, "Failed to find sitemap module.");
      break;
    }

    if (sitemap_index != -1) {
      VARIANT var_index;
      var_index.vt = VT_I2;
      var_index.iVal = sitemap_index;

      hresult = collection->DeleteElement(var_index);
      if (FAILED(hresult)) {
        Logger::Log(EVENT_ERROR, "Failed to remove sitemap module.");
      }
    } else {
      Logger::Log(EVENT_IMPORTANT, "No sitemap module is found.");
    }

    result = SUCCEEDED(hresult);
  } while (false);

  // Exiting / Unwinding
  if (name_property != NULL) name_property->Release();
  if (element != NULL) element->Release();
  if (collection != NULL) collection->Release();
  if (parent != NULL) parent->Release();

  SysFreeString(bstr_section_name);
  SysFreeString(bstr_config_path);

  return result;
}

bool Iis7Config::LoadSite(IAppHostElement* element) {
  std::string name, id, phys_path, log_path;

  // Get name.
  VARIANT name_var;
  if (!GetProperty(element, L"name", &name_var)) {
    Logger::Log(EVENT_ERROR, "Failed to get name property value.");
    return false;
  } else if (name_var.vt != VT_BSTR) {
    Logger::Log(EVENT_ERROR, "site name should be VT_BSTR");
    return false;
  } else {
    name = static_cast<const char*>(bstr_t(name_var.bstrVal));
  }

  // Get id.
  VARIANT id_var;
  if (!GetProperty(element, L"id", &id_var)) {
    Logger::Log(EVENT_ERROR, "Failed to get id value.");
    return false;
  } else if (id_var.vt == VT_I4) {
    char buffer[16];
    itoa(id_var.intVal, buffer, 10);
    id = buffer;
  } else if (id_var.vt == VT_UI4) {
    char buffer[16];
    itoa(id_var.uintVal, buffer, 10);
    id = buffer;
  } else if (id_var.vt == VT_BSTR) {
    id = static_cast<const char*>(bstr_t(id_var.bstrVal));
  } else {
    Logger::Log(EVENT_ERROR, "site id should be VT_BSTR or VT_I4");
  }

  // Get logfile directory.
  IAppHostElement* logfile_element = NULL;
  HRESULT hresult = element->GetElementByName(L"logfile", &logfile_element);
  if (FAILED(hresult)) {
    Logger::Log(EVENT_ERROR, "Failed to get logfile element.");
  } else {
    VARIANT dir_var;
    if (!GetProperty(logfile_element, L"directory", &dir_var)) {
      Logger::Log(EVENT_ERROR, "Failed to get directory value.");
    } else if (dir_var.vt == VT_BSTR) {
      log_path = static_cast<const char*>(bstr_t(dir_var.bstrVal));
      log_path.append("\\W3SVC").append(id);
    } else {
      Logger::Log(EVENT_ERROR, "directory should be VT_BSTR");
    }
  }

  // Get pysical path.
  IAppHostElementCollection* application_collection = NULL;
  IAppHostElementCollection* virtualdir_collection = NULL;
  IAppHostElement* application_element = NULL;
  IAppHostElement* virtualdir_element = NULL;

  hresult = S_OK;
  bool result = false;
  do {
    // Get <application> collection in <site> element
    hresult = element->get_Collection(&application_collection);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Failed to get application collection.");
      break;
    }

    // Find <application> element wiht path="/"
    short root_app = -1;
    if (!GetFromCollection(application_collection, L"path", L"/", &root_app)) {
      Logger::Log(EVENT_ERROR, "Failed to get root application index.");
      break;
    } else if (root_app == -1) {
      Logger::Log(EVENT_ERROR, "No root application defined.");
      break;
    }

    // Get the root <application> element
    VARIANT var_rootapp;
    var_rootapp.vt = VT_I2;
    var_rootapp.iVal = root_app;
    hresult = application_collection->get_Item(var_rootapp, &application_element);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Failed to get root application element.");
      break;
    }

    // Get <virtualDirectory> collection in <application> element.
    hresult = application_element->get_Collection(&virtualdir_collection);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Failed to get virtualdir collection.");
      break;
    }

    // Find <virtualDirectory> element with path="/"
    short root_dir = -1;
    if (!GetFromCollection(virtualdir_collection, L"path", L"/", &root_dir)) {
      Logger::Log(EVENT_ERROR, "Failed to get root virtualDirectory index.");
      break;
    } else if (root_dir == -1) {
      Logger::Log(EVENT_ERROR, "No root virtualDirectory defined.");
      break;
    }

    // Get the root <virtualDirectory> element.
    VARIANT var_rootdir;
    var_rootdir.vt = VT_I2;
    var_rootdir.iVal = root_dir;
    hresult = virtualdir_collection->get_Item(var_rootdir, &virtualdir_element);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Failed to get root virtualDirectory element.");
      break;
    }

    VARIANT var_path;
    if (!GetProperty(virtualdir_element, L"physicalPath", &var_path)) {
      Logger::Log(EVENT_ERROR, "Failed to get physicalPath property.");
      break;
    } else if (var_path.vt != VT_BSTR) {
      Logger::Log(EVENT_ERROR, "physicalPath should be instance of VT_BSTR.");
      break;
    }

    phys_path = static_cast<const char*>(bstr_t(var_path.bstrVal));
    result = true;
  } while (false);

  if (logfile_element != NULL) logfile_element->Release();
  if (application_collection != NULL) application_collection->Release();
  if (virtualdir_collection != NULL) virtualdir_collection->Release();
  if (application_element != NULL) application_element->Release();
  if (virtualdir_element != NULL) virtualdir_element->Release();

  // At last we store all the values.
  if (result) {
    site_ids_.push_back(id);
    names_.push_back(name);
    physical_paths_.push_back(phys_path);
    log_paths_.push_back(log_path);
    host_urls_.push_back("");
  }

  return result;
}

bool Iis7Config::GetFromCollection(IAppHostElementCollection* collection,
                                   BSTR property_key, BSTR property_value,
                                   short* index) {
    IAppHostProperty* property = NULL;
    IAppHostElement* element = NULL;

    HRESULT hresult = S_OK;
    *index = -1;
    do {
      // Get number of children.
      DWORD count;
      hresult = collection->get_Count(&count);
      if (FAILED(hresult)) {
        Logger::Log(EVENT_ERROR, "Unable to get the count of collection.");
        break;
      }

      // Iterate every child to check whether it should be removed.
      for (USHORT i = 0; i < count && SUCCEEDED(hresult); ++i) {
        // Get item at index i.
        VARIANT idx;
        idx.vt = VT_I2;
        idx.iVal = i;
        hresult = collection->get_Item(idx, &element);
        if (FAILED(hresult)) {
          Logger::Log(EVENT_ERROR, "Unable to get item (%d).", i);
          break;
        }

        // Get name property of item i.
        VARIANT var_value;
        if (!GetProperty(element, property_key, &var_value)) {
          Logger::Log(EVENT_ERROR, "Failed to get property value.");
          hresult = S_FALSE;
          break;
        }

        // Check the property value
        if (wcscmp(property_value, var_value.bstrVal) == 0) {
          *index = static_cast<short>(i);
          break;
        }

        element->Release();
        element = NULL;
      }
    } while (false);

    if (property != NULL) property->Release();
    if (element != NULL) element->Release();

    return SUCCEEDED(hresult);
}

bool Iis7Config::GetFromCollection(IAppHostElementCollection* collection,
                                   BSTR property_key, BSTR property_value,
                                   IAppHostElement** element) {
  short idx;
  if (!GetFromCollection(collection, property_key, property_value, &idx)) {
    Logger::Log(EVENT_ERROR, "Failed to get child from collection.");
  }

  if (idx != -1) {
    VARIANT idx_var;
    idx_var.vt = VT_I2;
    idx_var.iVal = idx;

    HRESULT hresult = collection->get_Item(idx_var, element);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Failed to get element from collection.");
      return false;
    } else {
      return true;
    }
  } else {
    *element = NULL;
    return true;
  }
}


bool Iis7Config::SetProperty(IAppHostElement* element, BSTR name, BSTR value) {
  IAppHostProperty* property = NULL;
  BSTR bstr_name = SysAllocString(name);
  BSTR bstr_value = SysAllocString(value);

  HRESULT hresult = S_OK;
  do {
	  // Get the property by name.
	  hresult = element->GetPropertyByName(bstr_name, &property);
	  if (FAILED(hresult)) {
		  Logger::Log(EVENT_ERROR, "Failed to get property.");
       break;
	  }

    // Set the property value.
    VARIANT value_variant;
    value_variant.vt = VT_BSTR;
    value_variant.bstrVal = bstr_value;
	  hresult = property->put_Value(value_variant);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Failed to set property value.");
	    break;
    }
  } while (false);

  if (property != NULL) property->Release();
  SysFreeString(bstr_name);
  SysFreeString(bstr_value);

  return SUCCEEDED(hresult);
}

bool Iis7Config::GetProperty(IAppHostElement* element, BSTR name, VARIANT* value) {
  IAppHostProperty* property = NULL;
  BSTR bstr_name = SysAllocString(name);

  HRESULT hresult = S_OK;
  do {
	  // Get the property by name.
	  hresult = element->GetPropertyByName(bstr_name, &property);
	  if (FAILED(hresult)) {
		  Logger::Log(EVENT_ERROR, "Failed to get property.");
       break;
	  }

    // Get the property value.
	  hresult = property->get_Value(value);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Failed to get property value.");
	    break;
    }
  } while (false);

  if (property != NULL) property->Release();
  SysFreeString(bstr_name);

  return SUCCEEDED(hresult);
}


bool Iis7Config::IsSupported() {
  HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
  if (FAILED(hr)) {
    Logger::Log(EVENT_ERROR, "Failed to check IIS 7 compatibility. 0x%0x", hr);
    return false;
  }

  // Try with IIS 7.0...
  IAppHostAdminManager* admin_manager = NULL;
  hr = CoCreateInstance(__uuidof(AppHostAdminManager), NULL, 
    CLSCTX_INPROC_SERVER, __uuidof(IAppHostAdminManager),
    reinterpret_cast<LPVOID*>(&admin_manager));
  if (SUCCEEDED(hr)) {
    admin_manager->Release();
  }

  CoUninitialize();

  return SUCCEEDED(hr);
}

IisConfig::AppMode Iis7Config::GetAppMode() {
  SYSTEM_INFO sys_info;
  if (IisConfig::GetNativeSystemInfo(&sys_info) != 1) {
    Logger::Log(EVENT_ERROR, "Failed to GetNativeSystemInfo for IIS7.");
    return MODE_FAIL;
  }

  if (sys_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
    return MODE_X86;
  } else if (sys_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
    return MODE_X64;
  } else {
    return MODE_UNKNOWN;
  }
}

bool Iis7Config::InstallAdminConsole(const char* site_root,
                                     const char* cgi_path) {
  IAppHostWritableAdminManager*   admin_manager = NULL;
  bool result = false;
  do {
    // Initialize com
    CHECK_HRESULT(CoInitializeEx(NULL, COINIT_MULTITHREADED), "Failed to intialize COM.");

    // Create a writable admin manager
    CHECK_HRESULT(CoCreateInstance(__uuidof(AppHostWritableAdminManager), NULL, 
              CLSCTX_INPROC_SERVER,
              __uuidof(IAppHostWritableAdminManager),
              reinterpret_cast<LPVOID*>(&admin_manager)),
              "Failed to create AppHostWritableAdminManager.");

    // set commit path
    CHECK_HRESULT(admin_manager->put_CommitPath(L"MACHINE/WEBROOT/APPHOST"),
                  "Failed to put commit path.");

    
    CHECK_BOOL(CreateAdminConsoleSite(admin_manager, bstr_t(site_root)),
               "Failed to create AdminConsole Site.");
    CHECK_BOOL(SetCGIRestriction(admin_manager, bstr_t(cgi_path)),
               "Failed to set CGI restriction.");
    CHECK_BOOL(CustomizeAdminConsole(admin_manager, bstr_t(cgi_path)),
               "Failed to customize admin console.");

    // Save changes.
    CHECK_HRESULT(admin_manager->CommitChanges(),
                  "Failed to save changes to install module.");

    Logger::Log(EVENT_CRITICAL, "Admin Console installed successfully.");
    result = true;
  } while (false);

  // Exiting / Unwinding
  if (admin_manager != NULL) admin_manager->Release();

  CoUninitialize();

  return result;
}

bool Iis7Config::UninstallAdminConsole(const char* site_root,
                                       const char* cgi_path) {
  IAppHostWritableAdminManager*   admin_manager = NULL;
  BSTR bstr_config_path = SysAllocString(L"MACHINE/WEBROOT/APPHOST");

  bool result = false;
  do {
    // Initialize com
    CHECK_HRESULT(CoInitializeEx(NULL, COINIT_MULTITHREADED), "Failed to intialize COM.");

    // Create a writable admin manager
    CHECK_HRESULT(CoCreateInstance(__uuidof(AppHostWritableAdminManager), NULL, 
              CLSCTX_INPROC_SERVER,
              __uuidof(IAppHostWritableAdminManager),
              reinterpret_cast<LPVOID*>(&admin_manager)),
              "Failed to create AppHostWritableAdminManager.");

    // set commit path
    CHECK_HRESULT(admin_manager->put_CommitPath(L"MACHINE/WEBROOT/APPHOST"),
                  "Failed to put commit path.");

    CHECK_BOOL(RemoveFromCollection(admin_manager, L"system.applicationHost/sites", L"name", kSiteName), "Failed to remove Admin console site.");
    CHECK_BOOL(RemoveFromCollection(admin_manager, L"system.webServer/security/isapiCgiRestriction", L"path", bstr_t(cgi_path)),
               "Failed to remove CGI restriction.");
    CHECK_BOOL(RemoveAdminConsoleCustomization(admin_manager),
               "Failed to remove customization of admin console.");

    // Save changes.
    CHECK_HRESULT(admin_manager->CommitChanges(),
                  "Failed to save changes to install module.");

    result = true;
  } while (false);

  // Exiting / Unwinding
  if (admin_manager != NULL) admin_manager->Release();

  SysFreeString(bstr_config_path);

  CoUninitialize();

  return result;
}



bool Iis7Config::SetCGIRestriction(IAppHostWritableAdminManager* manager, BSTR cgi_path) {
  IAppHostElement*        parent = NULL;
  IAppHostElementCollection* collection = NULL;
  IAppHostElement*        element = NULL;

  BSTR bstr_section_name = SysAllocString(L"system.webServer/security/isapiCgiRestriction");
  BSTR bstr_config_path = SysAllocString(L"MACHINE/WEBROOT/APPHOST");
  BSTR bstr_element_name = SysAllocString(L"add");

  HRESULT hresult = S_OK;
  bool result = false;
  do {
    // Get <system.webServer/security/isapiCgiRestriction> section
    hresult = manager->GetAdminSection(bstr_section_name,
                                       bstr_config_path, &parent);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Unable to access isapiCgiRestriction configuration. (0x%0x)", hresult);
      break;
    } else if (&parent == NULL) {
      Logger::Log(EVENT_ERROR, "Unable to get isapiCgiRestriction configuration."); 
      break;
    }

    // Get collection of <system.webServer/security/isapiCgiRestriction>
    hresult = parent->get_Collection(&collection);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Unable to access isapiCgiRestriction child collection. (0x%0x)", hresult);
      break;
    } else if (&collection == NULL) {
      Logger::Log(EVENT_ERROR, "Unable to get isapiCgiRestriction child collection."); 
      break;
    }

    // create a child element, like:
    // <add path="C:\Program Fils\GSG\AdminConsoleCGI.exe" />

    // First to detect old cgi restriction.
    if (!GetFromCollection(collection, L"path", cgi_path, &element)) {
      Logger::Log(EVENT_ERROR, "Failed to try detect old cgi restriction.");
      break;
    }

    // No old cgi restriction, create one.
    if (element == NULL) {
      hresult = collection->CreateNewElement(bstr_element_name, &element);
      if (FAILED(hresult)) {
        Logger::Log(EVENT_ERROR, "Failed to create isapiCgiRestriction/add element. (0x%0x)", hresult);
        break;
      }

      if (!SetProperty(element, L"path", cgi_path)) {
        Logger::Log(EVENT_ERROR, "Failed to set path property.");
        break;
      }
      
      if (!SetProperty(element, L"allowed", L"true")) {
        Logger::Log(EVENT_ERROR, "Failed to set allowed property.");
        break;
      }

      // Add the new element to collection
      hresult = collection->AddElement(element);
      if (FAILED(hresult)) {
        Logger::Log(EVENT_ERROR, "Failed to add isapiCgiRestriction/add element. (0x%0x)", hresult);
        break;
      }
    }

    Logger::Log(EVENT_NORMAL, "CGIRestriction set properly.");
    result = true;
  } while (false);

  // Exiting / Unwinding
  if (element != NULL) element->Release();
  if (collection != NULL) collection->Release();
  if (parent != NULL) parent->Release();

  SysFreeString(bstr_section_name);
  SysFreeString(bstr_config_path);
  SysFreeString(bstr_element_name);

  return result;
}


bool Iis7Config::CreateAdminConsoleSite(IAppHostWritableAdminManager *manager,  BSTR site_root) {
  IAppHostElement*        parent = NULL;
  IAppHostElementCollection* collection = NULL;
  IAppHostElement*        element = NULL;

  BSTR bstr_section_name = SysAllocString(L"system.applicationHost/sites");
  BSTR bstr_config_path = SysAllocString(L"MACHINE/WEBROOT/APPHOST");

  HRESULT hresult = S_OK;
  bool result = false;
  do {
    // Get <system.applicationHost/sites> section
    hresult = manager->GetAdminSection(bstr_section_name,
                                       bstr_config_path, &parent);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Unable to access sites configuration. (0x%0x)", hresult);
      break;
    } else if (&parent == NULL) {
      Logger::Log(EVENT_ERROR, "Unable to get sites configuration."); 
      break;
    }

    // Get collection of <system.applicationHost/sites>
    hresult = parent->get_Collection(&collection);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Unable to access sites child collection. (0x%0x)", hresult);
      break;
    } else if (&collection == NULL) {
      Logger::Log(EVENT_ERROR, "Unable to get sites child collection."); 
      break;
    }

    // create a <site> element, like:

    // First to detect old site.
    if (!GetFromCollection(collection, L"name", kSiteName, &element)) {
      Logger::Log(EVENT_ERROR, "Failed to try detect old site.");
      break;
    }
    if (element != NULL) {
      Logger::Log(EVENT_CRITICAL, "Old Admin Console Site exists. Skip.");
      result = true;
      break;
    }

    DWORD site_count;
    CHECK_HRESULT(collection->get_Count(&site_count),
                  "Failed to get site count.");
    char site_id[64];
    itoa(site_count + 466453, site_id, 10);

    CHECK_HRESULT(collection->CreateNewElement(L"site", &element),
                  "Failed to create site element.");
    CHECK_BOOL(SetProperty(element, L"name", kSiteName),
               "Failed to set site name property.");
    CHECK_BOOL(SetProperty(element, L"id", bstr_t(site_id)),
               "Failed to set site id property.");
    CHECK_HRESULT(collection->AddElement(element),
                  "Failed to add site element.");

    // Create and set <application> element.
    IAppHostElementCollection* app_collection = NULL;
    IAppHostElement* app_element = NULL;
    CHECK_HRESULT(element->get_Collection(&app_collection),
                  "Failed to get app collection.");
    CHECK_HRESULT(app_collection->CreateNewElement(L"application", &app_element), 
                  "Failed to create application element.");
    CHECK_BOOL(SetProperty(app_element, L"path", L"/"),
               "Failed to set app path.");
    // CHECK_BOOL(SetProperty(app_element, L"applicationPool", kSiteName), "Failed to set app applicationPool.");
    CHECK_HRESULT(app_collection->AddElement(app_element),
                  "Failed to add application element.");

    // Create and set <virtualDirectory> element.
    IAppHostElementCollection* dir_collection = NULL;
    IAppHostElement* dir_element = NULL;
    CHECK_HRESULT(app_element->get_Collection(&dir_collection),
                  "Failed to get virtualDirectory collection.");
    CHECK_HRESULT(dir_collection->CreateNewElement(L"virtualDirectory", &dir_element),
                  "Failed to create virtualDirectory.");
    CHECK_BOOL(SetProperty(dir_element, L"path", L"/"),
               "Failed to set dir path.");
    CHECK_BOOL(SetProperty(dir_element, L"physicalPath", site_root),
               "Failed to set dir physicalPath.");
    CHECK_HRESULT(dir_collection->AddElement(dir_element),
                  "Failed to add virtualDirectory.");

    // Get <bindings> element.
    IAppHostElement* bindings_element = NULL;
    CHECK_HRESULT(element->GetElementByName(L"bindings", &bindings_element),
                  "Failed to get bindings element.");
    CHECK_BOOL((bindings_element != NULL),
               "Failed to find bindings element.");

    // Create and set <binding> element.
    IAppHostElementCollection* binding_collection = NULL;
    IAppHostElement* binding_element = NULL;
    CHECK_HRESULT(bindings_element->get_Collection(&binding_collection),
                  "Failed to get binding collection.");
    CHECK_HRESULT(binding_collection->CreateNewElement(L"binding", &binding_element),
                  "Failed to create binding.");
    CHECK_BOOL(SetProperty(binding_element, L"protocol", L"http"),
               "Failed to set binding protocol.");
    CHECK_BOOL(SetProperty(binding_element, L"bindingInformation", bstr_t(IisConfig::kAdminConsoleBinding)),
               "Failed to set binding bindingInformation.");
    CHECK_HRESULT(binding_collection->AddElement(binding_element),
                  "Failed to add binding element.");

    Logger::Log(EVENT_NORMAL, "Admin Console site created successfully.");
    result = true;
  } while (false);

  // Exiting / Unwinding
  if (element != NULL) element->Release();
  if (collection != NULL) collection->Release();
  if (parent != NULL) parent->Release();

  SysFreeString(bstr_section_name);
  SysFreeString(bstr_config_path);

  return result;
}

bool Iis7Config::GetFromChildren(IAppHostChildElementCollection* children,
                                 BSTR child_name, short* index) {
  IAppHostElement* element = NULL;

  HRESULT hresult = S_OK;
  *index = -1;
  do {
    // Get number of children.
    DWORD count;
    hresult = children->get_Count(&count);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Unable to get the count of children.");
      break;
    }

    // Iterate every child to check whether it should be removed.
    for (USHORT i = 0; i < count && SUCCEEDED(hresult); ++i) {
      // Get item at index i.
      VARIANT idx;
      idx.vt = VT_I2;
      idx.iVal = i;
      hresult = children->get_Item(idx, &element);
      if (FAILED(hresult)) {
        Logger::Log(EVENT_ERROR, "Unable to get item [%d]. (0x%0x)", i, hresult);
        break;
      }

      // Get name property of item i.
      
      BSTR name = NULL;
      if (!element->get_Name(&name)) {
        Logger::Log(EVENT_ERROR, "Failed to get property value.");
        hresult = S_FALSE;
        break;
      }

      // Check the property value
      if (wcscmp(child_name, name) == 0) {
        *index = static_cast<short>(i);
        SysFreeString(name);
        break;
      }

      SysFreeString(name);
      element->Release();
      element = NULL;
    }
  } while (false);


  if (element != NULL) element->Release();

  return SUCCEEDED(hresult);  
}

bool Iis7Config::GetFromChildren(IAppHostChildElementCollection* children,
                                 BSTR child_name, IAppHostElement** element) {
  short idx;
  if (!GetFromChildren(children, child_name, &idx)) {
    Logger::Log(EVENT_ERROR, "Failed to get element from children.");
  }

  if (idx != -1) {
    VARIANT idx_var;
    idx_var.vt = VT_I2;
    idx_var.iVal = idx;

    HRESULT hresult = children->get_Item(idx_var, element);
    if (FAILED(hresult)) {
      Logger::Log(EVENT_ERROR, "Failed to get element from children.");
      return false;
    } else {
      return true;
    }
  } else {
    *element = NULL;
    return true;
  }
}

bool Iis7Config::CustomizeAdminConsole(IAppHostWritableAdminManager* manager, BSTR cgi_path) {
  IAppHostConfigManager* config_manager = NULL;
  IAppHostConfigFile * config_file = NULL;
  IAppHostConfigLocationCollection* location_collection = NULL;
  IAppHostConfigLocation* location = NULL;
  IAppHostElement* handlers = NULL;
  IAppHostElementCollection* handlers_collection = NULL;
  IAppHostElement* remove_element = NULL;
  IAppHostElement* add_element = NULL;

  bool result = false;
  do {
    CHECK_HRESULT(manager->get_ConfigManager(&config_manager),
                  "Failed to get config manager.");
    CHECK_HRESULT(config_manager->GetConfigFile(L"MACHINE/WEBROOT/APPHOST", &config_file),
                  "Failed to get config file.");
    CHECK_HRESULT(config_file->get_Locations(&location_collection),
                  "Failed to get location collection.");

    bool success = false;
    do {
      // Get number of children.
      DWORD count;
      CHECK_HRESULT(location_collection->get_Count(&count), "Unable to get the count of collection.");

      // Iterate every child to check whether it should be removed.
      USHORT i = 0;
      for (; i < count; ++i) {
        // Get item at index i.
        VARIANT idx;
        idx.vt = VT_I2;
        idx.iVal = i;
        CHECK_HRESULT(location_collection->get_Item(idx, &location), "Unable to get item.");

        BSTR location_path;
        CHECK_HRESULT(location->get_Path(&location_path), "Failed to get location path.");
        if (wcscmp(location_path, kSiteName) == 0) {
          success = true;
          break;
        }

        SysFreeString(location_path);
        location->Release();
        location = NULL;
      }

      if (i == count) success = true;
    } while (false);

    if (!success) {
      break;
    }

    if (location != NULL) {
      Logger::Log(EVENT_CRITICAL, "Location already exists. Skip.");
      result = true;
      break;
    }

    CHECK_HRESULT(location_collection->AddLocation(kSiteName, &location),
                  "Failed to add location.");
    CHECK_HRESULT(location->AddConfigSection(L"system.webServer/handlers", &handlers),
                  "Failed to add config section."); 
    CHECK_HRESULT(SetProperty(handlers, L"accessPolicy", L"Read, Execute, Script"),
                  "Failed to set accessPolicy.");

    CHECK_HRESULT(handlers->get_Collection(&handlers_collection),
                  "Failed to get handlers collection.");
    CHECK_HRESULT(handlers_collection->CreateNewElement(L"remove", &remove_element),
                  "Failed to create remove element.");
    CHECK_BOOL(SetProperty(remove_element, L"name", L"CGI-exe"),
               "Failed to set name of remove element.");
    CHECK_HRESULT(handlers_collection->AddElement(remove_element),
                  "Failed to add remove element.");
    
    CHECK_HRESULT(handlers_collection->CreateNewElement(L"add", &add_element),
                  "Failed to create add element.");
    CHECK_BOOL(SetProperty(add_element, L"name", L"CGI-exe"),
               "Failed to set name of add element.");
    CHECK_BOOL(SetProperty(add_element, L"path", L"*.cgi"),
               "Failed to set path of add element.");
    CHECK_BOOL(SetProperty(add_element, L"verb", L"*"),
               "Failed to set verb of  element.");
    CHECK_BOOL(SetProperty(add_element, L"modules", L"CgiModule"),
               "Failed to set modules of add element.");
    CHECK_BOOL(SetProperty(add_element, L"scriptProcessor", cgi_path),
               "Failed to set scriptProcessor of add element.");
    CHECK_BOOL(SetProperty(add_element, L"resourceType", L"File"),
               "Failed to set resourceType of add element.");
    CHECK_BOOL(SetProperty(add_element, L"requireAccess", L"Execute"),
               "Failed to set requireAccess of add element.");
    CHECK_BOOL(SetProperty(add_element, L"allowPathInfo", L"true"),
               "Failed to set naallowPathInfome of add element.");
    CHECK_HRESULT(handlers_collection->AddElement(add_element),
                  "Failed to add add element.");

    Logger::Log(EVENT_NORMAL, "Admin Console site customized successfully.");
    result = true;
  } while (false);

  return result;
}


bool Iis7Config::RemoveAdminConsoleCustomization(IAppHostWritableAdminManager* manager) {
  IAppHostConfigManager* config_manager = NULL;
  IAppHostConfigFile * config_file = NULL;
  IAppHostConfigLocationCollection* location_collection = NULL;
  IAppHostConfigLocation* location = NULL;

  bool result = false;
  do {
    CHECK_HRESULT(manager->get_ConfigManager(&config_manager), "Failed to get config manager.");
    CHECK_HRESULT(config_manager->GetConfigFile(L"MACHINE/WEBROOT/APPHOST", &config_file), "Failed to get config file.");
    CHECK_HRESULT(config_file->get_Locations(&location_collection), "Failed to get location collection.");

    bool success = false;
    VARIANT idx;
    do {
      // Get number of children.
      DWORD count;
      CHECK_HRESULT(location_collection->get_Count(&count), "Unable to get the count of collection.");

      // Iterate every child to check whether it should be removed.
      USHORT i = 0;
      for (; i < count; ++i) {
        // Get item at index i.
        idx.vt = VT_I2;
        idx.iVal = i;
        CHECK_HRESULT(location_collection->get_Item(idx, &location), "Unable to get item.");

        BSTR location_path;
        CHECK_HRESULT(location->get_Path(&location_path), "Failed to get location path.");
        if (wcscmp(location_path, kSiteName) == 0) {
          success = true;
          break;
        }

        SysFreeString(location_path);
        location->Release();
        location = NULL;
      }

      if (i == count) success = true;
    } while (false);

    if (!success) {
      break;
    }

    if (location == NULL) {
      Logger::Log(EVENT_CRITICAL, "No location exists. Skip.");
      result = true;
      break;
    }

    CHECK_HRESULT(location_collection->DeleteLocation(idx), "Failed to remove location.");

    result = true;
  } while (false);

  return result;
}



