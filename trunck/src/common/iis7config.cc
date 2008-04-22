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
#include "common/util.h"

BSTR Iis7Config::kModuleName = L"GoogleSitemapGeneratorModule";

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
      Util::Log(EVENT_ERROR, "Failed to intialize COM.");
      break;
    }

    // Create an admin manager
    hresult = CoCreateInstance(__uuidof(AppHostAdminManager), NULL, 
              CLSCTX_INPROC_SERVER,
              __uuidof(IAppHostAdminManager),
              reinterpret_cast<LPVOID*>(&admin_manager));
    if (FAILED(hresult)) {
      Util::Log(EVENT_ERROR, "Failed to create AppHostAdminManager.");
      break;
    }

    // Get <sites> section
    hresult = admin_manager->GetAdminSection(section_name, bstr_config_path, &parent);
    if (FAILED(hresult)) {
      Util::Log(EVENT_ERROR, "Unable to access sites configuration.");
      break;
    } else if (&parent == NULL) {
      Util::Log(EVENT_ERROR, "Unable to get sites configuration."); 
      break;
    }

    // Get all the child elements of <sites>
    hresult = parent->get_Collection(&collection);
    if (FAILED(hresult)) {
      Util::Log(EVENT_ERROR, "Unable to access <sites> child collection.");
      break;
    } else if (&collection == NULL) {
      Util::Log(EVENT_ERROR, "Unable to get <sites> child collection."); 
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
        Util::Log(EVENT_ERROR, "Failed to iterate child (%d) of sites.", i);
        continue;
      }

      if (!LoadSite(element)) {
        Util::Log(EVENT_ERROR, "Failed to load site from site (%d).", i);
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
      Util::Log(EVENT_ERROR, "Failed to intialize COM.");
      break;
    }

    // Create a writable admin manager
    hresult = CoCreateInstance(__uuidof(AppHostWritableAdminManager), NULL, 
              CLSCTX_INPROC_SERVER,
              __uuidof(IAppHostWritableAdminManager),
              reinterpret_cast<LPVOID*>(&admin_manager));
    if (FAILED(hresult)) {
      Util::Log(EVENT_ERROR, "Failed to create AppHostWritableAdminManager.");
      break;
    }

    // set commit path
    hresult = admin_manager->put_CommitPath(L"MACHINE/WEBROOT/APPHOST");
    if (FAILED(hresult)) {
      Util::Log(EVENT_ERROR, "Failed to put commit path.");
      break;
    }

    // Add sitemap entry to <system.webServer/globalModules> section
    if (!AddToGlobalModules(admin_manager, module_wpath)) {
      Util::Log(EVENT_ERROR, "Failed to add entry to globalModules.");
      break;
    }

    // Add sitemap entry to <system.webServer/modules> section
    if (!AddToModules(admin_manager)) {
      Util::Log(EVENT_ERROR, "Failed to add entry to modules.");
      break;
    }

    // Save changes.
    hresult = admin_manager->CommitChanges();
    if (FAILED(hresult)) {
      Util::Log(EVENT_ERROR, "Failed to save changes to install module.");
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
      Util::Log(EVENT_ERROR, "Failed to intialize COM.");
      break;
    }

    // Create a writable admin manager
    hresult = CoCreateInstance(__uuidof(AppHostWritableAdminManager), NULL, 
              CLSCTX_INPROC_SERVER,
              __uuidof(IAppHostWritableAdminManager),
              reinterpret_cast<LPVOID*>(&admin_manager));
    if (FAILED(hresult)) {
      Util::Log(EVENT_ERROR, "Failed to create AppHostWritableAdminManager.");
      break;
    }

    // set commit path
    hresult = admin_manager->put_CommitPath(bstr_config_path);
    if (FAILED(hresult)) {
      Util::Log(EVENT_ERROR, "Failed to put commit path.");
      break;
    }

    // Remove sitemap entry from <system.webServer/globalModules> section.
    if (!RemoveSitemapModule(admin_manager, L"system.webServer/globalModules")) {
      Util::Log(EVENT_ERROR, "Failed to remove entry from globalModules.");
      break;
    }

    // Remove sitemap entry from <system.webServer/modules> section.
    if (!RemoveSitemapModule(admin_manager, L"system.webServer/modules")) {
      Util::Log(EVENT_ERROR, "Failed to remove entry from modules.");
      break;
    }

    // Save changes.
    hresult = admin_manager->CommitChanges();
    if (FAILED(hresult)) {
      Util::Log(EVENT_ERROR, "Failed to save changes to install module.");
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
      Util::Log(EVENT_ERROR, "Unable to access globalModules configuration.");
      break;
    } else if (&parent == NULL) {
      Util::Log(EVENT_ERROR, "Unable to get globalModules configuration."); 
      break;
    }

    // Get collection of <system.webServer/globalModules>
    hresult = parent->get_Collection(&collection);
    if (FAILED(hresult)) {
      Util::Log(EVENT_ERROR, "Unable to access globalModules child collection.");
      break;
    } else if (&collection == NULL) {
      Util::Log(EVENT_ERROR, "Unable to get globalModules child collection."); 
      break;
    }

    // create a global modules child element, like:
    // <add name="GoogleSitemapGeneratorModule", image="sitemap_module.dll" />

    // First to detect old modules.
    if (!GetFromCollection(collection, L"name", kModuleName, &element)) {
      Util::Log(EVENT_ERROR, "Failed to try detect old modules.");
      break;
    }

    // No old sitemap module, create/add one.
    if (element == NULL) {
      hresult = collection->CreateNewElement(bstr_element_name, &element);
      if (FAILED(hresult)) {
        Util::Log(EVENT_ERROR, "Failed to create globalModules/add element.");
        break;
      }

      if (!SetProperty(element, L"name", kModuleName)) {
        Util::Log(EVENT_ERROR, "Failed to set name property.");
        break;
      }
      // Add the new element to collection
      hresult = collection->AddElement(element);
      if (FAILED(hresult)) {
        Util::Log(EVENT_ERROR, "Failed to add globalModule/add element.");
        break;
      }
    }

    if (!SetProperty(element, L"image", image)) {
      Util::Log(EVENT_ERROR, "Failed to set image property.");
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
      Util::Log(EVENT_ERROR, "Unable to access modules configuration.");
      break;
    } else if (&parent == NULL) {
      Util::Log(EVENT_ERROR, "Unable to get modules configuration."); 
      break;
    }

    // Get collection of <system.webServer/modules>
    hresult = parent->get_Collection(&collection);
    if (FAILED(hresult)) {
      Util::Log(EVENT_ERROR, "Unable to access modules child collection.");
      break;
    } else if (&collection == NULL) {
      Util::Log(EVENT_ERROR, "Unable to get modules child collection."); 
      break;
    }

    // create a modules child element, like:
    // <add name="GoogleSitemapGeneratorModule" />

    // Try to detect old sitemap module.
    if (!GetFromCollection(collection, L"name", kModuleName, &element)) {
      Util::Log(EVENT_ERROR, "Failed to try detect old modules.");
      break;
    }

    // No old sitemap module exists.
    if (element == NULL) {
      hresult = collection->CreateNewElement(bstr_element_name, &element);
      if (FAILED(hresult)) {
        Util::Log(EVENT_ERROR, "Failed to create modules/add element.");
        break;
      }

      if (!SetProperty(element, L"name", kModuleName)) {
        Util::Log(EVENT_ERROR, "Failed to set name property.");
        break;
      }

      // Add the new element to collection
      hresult = collection->AddElement(element);
      if (FAILED(hresult)) {
        Util::Log(EVENT_ERROR, "Failed to add modules/add element.");
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

bool Iis7Config::RemoveSitemapModule(IAppHostWritableAdminManager* manager,
                                     BSTR section) {
  IAppHostElement*        parent = NULL;
  IAppHostElementCollection* collection = NULL;
  IAppHostElement*        element = NULL;
  IAppHostProperty*       name_property = NULL;

  BSTR bstr_section_name = SysAllocString(section);
  BSTR bstr_config_path = SysAllocString(L"MACHINE/WEBROOT/APPHOST");
  BSTR bstr_name_key = SysAllocString(L"name");

  HRESULT hresult = S_OK;
  bool result = false;
  do {
    // Get section contains SitemapModule entry
    hresult = manager->GetAdminSection(bstr_section_name,
                                       bstr_config_path, &parent);
    if (FAILED(hresult)) {
      Util::Log(EVENT_ERROR, "Unable to access section to remove module.");
      break;
    } else if (&parent == NULL) {
      Util::Log(EVENT_ERROR, "Unable to get section to remove module."); 
      break;
    }

    // Get child collection of the section
    hresult = parent->get_Collection(&collection);
    if (FAILED(hresult)) {
      Util::Log(EVENT_ERROR, "Unable to access collection to remove module.");
      break;
    } else if (&collection == NULL) {
      Util::Log(EVENT_ERROR, "Unable to get collection to remove module."); 
      break;
    }

    // Get number of children.
    DWORD count;
    hresult = collection->get_Count(&count);
    if (FAILED(hresult)) {
      Util::Log(EVENT_ERROR, "Unable to get the count of collection.");
      break;
    }

    short sitemap_index;
    if (!GetFromCollection(collection, L"name", kModuleName, &sitemap_index)) {
      Util::Log(EVENT_ERROR, "Failed to find sitemap module.");
      break;
    }

    if (sitemap_index != -1) {
      VARIANT var_index;
      var_index.vt = VT_I2;
      var_index.iVal = sitemap_index;

      hresult = collection->DeleteElement(var_index);
      if (FAILED(hresult)) {
        Util::Log(EVENT_ERROR, "Failed to remove sitemap module.");
      }
    } else {
      Util::Log(EVENT_IMPORTANT, "No sitemap module is found.");
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
  SysFreeString(bstr_name_key);

  return result;
}

bool Iis7Config::LoadSite(IAppHostElement* element) {
  std::string name, id, phys_path, log_path;

  // Get name.
  VARIANT name_var;
  if (!GetProperty(element, L"name", &name_var)) {
    Util::Log(EVENT_ERROR, "Failed to get name property value.");
    return false;
  } else if (name_var.vt != VT_BSTR) {
    Util::Log(EVENT_ERROR, "site name should be VT_BSTR");
    return false;
  } else {
    name = static_cast<const char*>(bstr_t(name_var.bstrVal));
  }

  // Get id.
  VARIANT id_var;
  if (!GetProperty(element, L"id", &id_var)) {
    Util::Log(EVENT_ERROR, "Failed to get id value.");
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
    Util::Log(EVENT_ERROR, "site id should be VT_BSTR or VT_I4");
  }

  // Get logfile directory.
  IAppHostElement* logfile_element = NULL;
  HRESULT hresult = element->GetElementByName(L"logfile", &logfile_element);
  if (FAILED(hresult)) {
    Util::Log(EVENT_ERROR, "Failed to get logfile element.");
  } else {
    VARIANT dir_var;
    if (!GetProperty(logfile_element, L"directory", &dir_var)) {
      Util::Log(EVENT_ERROR, "Failed to get directory value.");
    } else if (dir_var.vt == VT_BSTR) {
      log_path = static_cast<const char*>(bstr_t(dir_var.bstrVal));
      log_path.append("\\W3SVC").append(id);
    } else {
      Util::Log(EVENT_ERROR, "directory should be VT_BSTR");
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
      Util::Log(EVENT_ERROR, "Failed to get application collection.");
      break;
    }

    // Find <application> element wiht path="/"
    short root_app = -1;
    if (!GetFromCollection(application_collection, L"path", L"/", &root_app)) {
      Util::Log(EVENT_ERROR, "Failed to get root application index.");
      break;
    } else if (root_app == -1) {
      Util::Log(EVENT_ERROR, "No root application defined.");
      break;
    }

    // Get the root <application> element
    VARIANT var_rootapp;
    var_rootapp.vt = VT_I2;
    var_rootapp.iVal = root_app;
    hresult = application_collection->get_Item(var_rootapp, &application_element);
    if (FAILED(hresult)) {
      Util::Log(EVENT_ERROR, "Failed to get root application element.");
      break;
    }

    // Get <virtualDirectory> collection in <application> element.
    hresult = application_element->get_Collection(&virtualdir_collection);
    if (FAILED(hresult)) {
      Util::Log(EVENT_ERROR, "Failed to get virtualdir collection.");
      break;
    }

    // Find <virtualDirectory> element with path="/"
    short root_dir = -1;
    if (!GetFromCollection(virtualdir_collection, L"path", L"/", &root_dir)) {
      Util::Log(EVENT_ERROR, "Failed to get root virtualDirectory index.");
      break;
    } else if (root_dir == -1) {
      Util::Log(EVENT_ERROR, "No root virtualDirectory defined.");
      break;
    }

    // Get the root <virtualDirectory> element.
    VARIANT var_rootdir;
    var_rootdir.vt = VT_I2;
    var_rootdir.iVal = root_dir;
    hresult = virtualdir_collection->get_Item(var_rootdir, &virtualdir_element);
    if (FAILED(hresult)) {
      Util::Log(EVENT_ERROR, "Failed to get root virtualDirectory element.");
      break;
    }

    VARIANT var_path;
    if (!GetProperty(virtualdir_element, L"physicalPath", &var_path)) {
      Util::Log(EVENT_ERROR, "Failed to get physicalPath property.");
      break;
    } else if (var_path.vt != VT_BSTR) {
      Util::Log(EVENT_ERROR, "physicalPath should be instance of VT_BSTR.");
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
        Util::Log(EVENT_ERROR, "Unable to get the count of collection.");
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
          Util::Log(EVENT_ERROR, "Unable to get item (%d).", i);
          break;
        }

        // Get name property of item i.
        VARIANT var_value;
        if (!GetProperty(element, property_key, &var_value)) {
          Util::Log(EVENT_ERROR, "Failed to get property value.");
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
    Util::Log(EVENT_ERROR, "Failed to get child from collection.");
  }

  if (idx != -1) {
    VARIANT idx_var;
    idx_var.vt = VT_I2;
    idx_var.iVal = idx;

    HRESULT hresult = collection->get_Item(idx_var, element);
    if (FAILED(hresult)) {
      Util::Log(EVENT_ERROR, "Failed to get element from collection.");
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
		  Util::Log(EVENT_ERROR, "Failed to get property.");
       break;
	  }

    // Set the property value.
    VARIANT value_variant;
    value_variant.vt = VT_BSTR;
    value_variant.bstrVal = bstr_value;
	  hresult = property->put_Value(value_variant);
    if (FAILED(hresult)) {
      Util::Log(EVENT_ERROR, "Failed to set property value.");
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
		  Util::Log(EVENT_ERROR, "Failed to get property.");
       break;
	  }

    // Get the property value.
	  hresult = property->get_Value(value);
    if (FAILED(hresult)) {
      Util::Log(EVENT_ERROR, "Failed to get property value.");
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
    Util::Log(EVENT_ERROR, "Failed to check IIS 7 compatibility. 0x%0x", hr);
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
  GetNativeSystemInfo(&sys_info);
  if (sys_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
    return MODE_X86;
  } else if (sys_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
    return MODE_X64;
  } else {
    return MODE_UNKNOWN;
  }
}

