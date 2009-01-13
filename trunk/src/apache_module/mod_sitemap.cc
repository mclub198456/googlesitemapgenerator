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


#include "apache_module/sitemapmodule.h"

#include "httpd.h"
#include "http_protocol.h"

#if APACHE_VERSION < 20
#define apr_pool_t pool
#endif

static const char *kModuleInitFlag = "google_sitemap_module_init";
static SitemapModule* sitemap_module = NULL;

static void module_init() __attribute__((__constructor__));
static void module_init() {
  // do nothing.
}

static void module_exit() __attribute__((__destructor__));
static void module_exit() {
  if (sitemap_module != NULL) {
    delete sitemap_module;
    sitemap_module = NULL;
  }
}

static void* create_server_config(apr_pool_t *p, server_rec *s) {
  // this piece of code only run under apache 2.0 and 2.2
  // It is used to ensure the module is only be initialized for the second time.
  // For apache1.3, currently, I don't have a good solution on it.
  // if EAPI is define, ap_ctx_set/ap_ctx are tried to be invoked with s->ctx,
  // but it doesn't work.
  // Anyway, creating module two times doens't hurt the application.
#if APACHE_VERSION >= 20
  void *userdata = NULL;
  apr_pool_userdata_get(&userdata, kModuleInitFlag,
                        s->process->pool);

  if (userdata == NULL) {
    if (apr_pool_userdata_set((const void*) 1, kModuleInitFlag,
                               apr_pool_cleanup_null, s->process->pool)
                               != APR_SUCCESS) {
      fprintf(stderr, "SitemapModule: can't set init flag.\n");
    }
    return NULL;
  }
#endif

  // for apache 2.x: following code is only runned in 2nd time
  sitemap_module = new SitemapModule();
  if (sitemap_module->Initialize() == false) {
    delete sitemap_module;
    sitemap_module = NULL;
    fprintf(stderr, "SitemapModule: can't initialize sitemap module.\n");
  }

  return NULL;
}

static int sitemap_logger(request_rec* req) {
  if (sitemap_module != NULL) {
    return sitemap_module->Process(req);
  }

  return DECLINED;
}

#if APACHE_VERSION >= 20
static void register_hooks(apr_pool_t *p) {
  ap_hook_log_transaction(&sitemap_logger, NULL, NULL, APR_HOOK_FIRST);
}

module AP_MODULE_DECLARE_DATA google_sitemap_generator_module = {
  STANDARD20_MODULE_STUFF,
  NULL,
  NULL,
  &create_server_config,
  NULL,
  NULL,
  &register_hooks,
};
#else
module MODULE_VAR_EXPORT google_sitemap_generator_module = {
  STANDARD_MODULE_STUFF,
  NULL, // &init,
  NULL,
  NULL,
  &create_server_config,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  &sitemap_logger,
  NULL
};
#endif
