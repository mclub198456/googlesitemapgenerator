/* Copyright 2002-2005 The Apache Software Foundation or its licensors, as
 * applicable.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file  ap_config_layout.h
 * @brief Apache Config Layout
 */

#ifndef AP_CONFIG_LAYOUT_H
#define AP_CONFIG_LAYOUT_H

/* Configured Apache directory layout */
#define DEFAULT_PREFIX "/etc/httpd"
#define DEFAULT_EXP_EXEC_PREFIX "/usr"
#define DEFAULT_REL_EXEC_PREFIX "/usr"
#define DEFAULT_EXP_BINDIR "/usr/bin"
#define DEFAULT_REL_BINDIR "/usr/bin"
#define DEFAULT_EXP_SBINDIR "/usr/sbin"
#define DEFAULT_REL_SBINDIR "/usr/sbin"
#define DEFAULT_EXP_LIBEXECDIR "/usr/lib64/httpd/modules"
#define DEFAULT_REL_LIBEXECDIR "/usr/lib64/httpd/modules"
#define DEFAULT_EXP_MANDIR "/usr/share/man"
#define DEFAULT_REL_MANDIR "/usr/share/man"
#define DEFAULT_EXP_SYSCONFDIR "/etc/httpd/conf"
#define DEFAULT_REL_SYSCONFDIR "conf"
#define DEFAULT_EXP_DATADIR "/var/www"
#define DEFAULT_REL_DATADIR "/var/www"
#define DEFAULT_EXP_INSTALLBUILDDIR "/var/www/build"
#define DEFAULT_REL_INSTALLBUILDDIR "/var/www/build"
#define DEFAULT_EXP_ERRORDIR "/var/www/error"
#define DEFAULT_REL_ERRORDIR "/var/www/error"
#define DEFAULT_EXP_ICONSDIR "/var/www/icons"
#define DEFAULT_REL_ICONSDIR "/var/www/icons"
#define DEFAULT_EXP_HTDOCSDIR "/var/www/htdocs"
#define DEFAULT_REL_HTDOCSDIR "/var/www/htdocs"
#define DEFAULT_EXP_MANUALDIR "/var/www/manual"
#define DEFAULT_REL_MANUALDIR "/var/www/manual"
#define DEFAULT_EXP_CGIDIR "/var/www/cgi-bin"
#define DEFAULT_REL_CGIDIR "/var/www/cgi-bin"
#define DEFAULT_EXP_INCLUDEDIR "/usr/include/httpd"
#define DEFAULT_REL_INCLUDEDIR "/usr/include/httpd"
#define DEFAULT_EXP_LOCALSTATEDIR "/etc/httpd"
#define DEFAULT_REL_LOCALSTATEDIR ""
#define DEFAULT_EXP_RUNTIMEDIR "/etc/httpd/logs"
#define DEFAULT_REL_RUNTIMEDIR "logs"
#define DEFAULT_EXP_LOGFILEDIR "/etc/httpd/logs"
#define DEFAULT_REL_LOGFILEDIR "logs"
#define DEFAULT_EXP_PROXYCACHEDIR "/etc/httpd/proxy"
#define DEFAULT_REL_PROXYCACHEDIR "proxy"

#endif /* AP_CONFIG_LAYOUT_H */
