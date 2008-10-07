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


#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/stat.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <utility>
#include <set>
#include <vector>
#include <string>

#include "common/port.h"
#include "common/logger.h"
#include "common/sitesettings.h"

#include "common/apacheconfig.h"

// default value is empty
std::string ApacheConfig::conf_file_path_ = "";

// a FQDN must be returned. (See RFC 4703)
// Idea comes from $ap_get_local_host in apache main source.
std::string ApacheConfig::GetLocalHost() {
  char buffer[1024];

  if (gethostname(buffer, sizeof(buffer) - 1) != 0) {
    Logger::Log(EVENT_ERROR, "Failed to get local host name.");
    return std::string("127.0.0.1");
  }

  buffer[sizeof(buffer) - 1] = '\0';
  struct hostent* host_entry = gethostbyname(buffer);
  if (host_entry == NULL) {
    Logger::Log(EVENT_ERROR, "Failed to get local host address.");
    return std::string("127.0.0.1");
  }

  // Get name of local host.
  if (host_entry->h_name != NULL && strchr(host_entry->h_name, '.') != NULL) {
    return std::string(host_entry->h_name);
  }

  for (char** p = host_entry->h_aliases; p != NULL && *p != NULL; ++p) {
    if (strchr(*p, '.') != NULL && (host_entry->h_name == NULL ||
        strncasecmp(*p, host_entry->h_name, strlen(host_entry->h_name)) == 0)) {
      return std::string(*p);
    }
  }

  if (host_entry->h_addr_list[0] != NULL) {
    char* ip = inet_ntoa(*reinterpret_cast<in_addr*>(host_entry->h_addr));
    if (ip != NULL) {
      return std::string(ip);
    }
  }

  Logger::Log(EVENT_ERROR, "127.0.0.1 is used as default servername at last.");
  return std::string("127.0.0.1");
}

// read a line, which is:
// 1) without comments, 2) with consideration of "\\"
bool ApacheConfig::Readline(std::ifstream* fout, std::string* line) {
  line->clear();
  std::string buffer;
  while (getline(*fout, buffer)) {
    // Someone may edit the configuration file under windows.
    std::string::size_type pos;
    while ((pos = buffer.find('\r')) != std::string::npos) {
      buffer.erase(pos, 1);
    }

    if (buffer.length() > 0 && buffer[buffer.length() - 1] == '\\') {
      line->append(buffer.substr(0, buffer.length() - 1));
    } else {
      line->append(buffer);
      if (line->length() != 0) break;
    }
  }

  // the file end must be reached
  if (line->length() == 0) {
    return false;
  }

  // find the place of '#', for comment
  int i = 0;
  while (i < line->length() && (*line)[i] != '#' && isspace((*line)[i])) {
    if (isspace((*line)[i]) != 0) {
      ++i;
    } else if ((*line)[i] == '#') {
      line->clear();
    } else {
      break;
    }
  }

  return true;
}

// Split by space, with consideration of quote
// WARN: Currently, no apache official doc on this issue,
//       idea from ${apache2-src}/server/util.c#ap_getword_conf)
std::vector<std::string> ApacheConfig::Split(const std::string& str) {
  std::vector<std::string> result;
  int len = static_cast<int>(str.length());

  for (int i = 0; i < len;) {
    while (i < len && isspace(str[i])) ++i;
    if (i == len) break;

    int j;
    if (str[i] == '\'' || str[i] == '"') {
      char quote = str[i++];
      j = i;
      while (j < len && str[j] != quote) {
        if (str[j] == '\\' && j + 1 < len) {
          // skip escape character '\\'
          j += 2;
        } else {
          ++j;
        }
      }
    } else {
      j = i;
      while (j < len && !isspace(str[j])) ++j;
    }

    result.push_back(str.substr(i, j - i));
    i = j + 1;
  }
  return result;
}


bool ApacheConfig::Load() {
  if (conf_file_path_.length() == 0) {
    Logger::Log(EVENT_ERROR, "Apache config file path shouldn't be empty.");
    return false;
  }

  return Load(conf_file_path_.c_str());
}

bool ApacheConfig::Load(const char* configfile) {
  site_ids_.clear();
  names_.clear();
  physical_paths_.clear();
  host_urls_.clear();

  vhosts_.clear();
  if (!ProcessFile(configfile)) {
    Logger::Log(EVENT_ERROR, "Can't load configuration properly");
    return false;
  }

  // Try to parse group_name_ to group_id_.
  if (group_name_.length() == 0) {
    group_name_ = "www-data";
  }

  // set default values for mainserver
  if (mainserver_.servername.length() == 0) {
    mainserver_.servername = GetLocalHost();
  }
  if (mainserver_.documentroot.length() == 0) {
    mainserver_.documentroot = "/htdocs";
  }
  mainserver_.address = ":*";

  // push mainserver to handle it in a normal way...
  vhosts_.push_back(mainserver_);

  // dump vhosts configurations
#ifdef _DEBUG
  for (int i = 0; i < static_cast<int>(vhosts_.size()); ++i) {
    fprintf(stderr, "  @ %s\t%s\t%s\n",
            vhosts_[i].servername.c_str(), vhosts_[i].documentroot.c_str(),
            vhosts_[i].address.c_str());
  }
#endif

  std::vector<VHostConfig>::iterator itr = vhosts_.begin();
  for (; itr != vhosts_.end(); ++itr) {
    VHostConfig conf = *itr;
    // Set document root.
    if (conf.documentroot.length() == 0) {
      conf.documentroot = mainserver_.documentroot;
    }
    if (!MakeAbsolutePath(&conf.documentroot)) {
      continue;
    }

    // Set custom log.
    if (conf.customlog.length() == 0) {
      conf.customlog = mainserver_.customlog;
    }
    if (conf.customlog.length() != 0) {
      if (!MakeAbsolutePath(&conf.customlog)) {
        continue;
      }
    }

    std::string siteid;
    if (BuildSiteId(&conf, &siteid, 0)) {
      site_ids_.push_back(siteid);
      names_.push_back(conf.servername);
      physical_paths_.push_back(conf.documentroot);
      host_urls_.push_back("");  // TODO: this field should be removed
      log_paths_.push_back(conf.customlog);
    }
  }

  // pop main server to avoid break the meaning of vhosts
  vhosts_.pop_back();

  return true;
}

// if it is relative, it should be made as absolute one with serverroot_
bool ApacheConfig::MakeAbsolutePath(std::string* path) {
  if (path->length() == 0) {
    return false;
  }

  if ((*path)[0] != '/') {
    if (serverroot_.length() == 0) {
      return false;
    } else {
      *path = serverroot_ + "/" + *path;
    }
  }
  return true;
}

// process a single configuration file
bool ApacheConfig::ProcessFile(const char* filename) {
  std::ifstream fout(filename);
  if (fout.fail()) {
    Logger::Log(EVENT_ERROR, "Failed to open apache conf file [%s]", filename);
    return false;
  }

  std::string line;

  VHostConfig* currentvhost = &mainserver_;

  while (Readline(&fout, &line)) {
    std::vector<std::string> parts = Split(line);
    if (parts.size() == 0) continue;   // skip empty line

    if (strcasecmp(parts[0].c_str(), "ServerRoot") == 0) {
      if (parts.size() == 2) {
        serverroot_ = parts[1];
      }
    } else if (strcasecmp(parts[0].c_str(), "ServerName") == 0) {
      if (currentvhost != NULL && parts.size() == 2) {
        std::string servername = parts[1];
        if (NormalizeServerName(&servername)) {
          currentvhost->servername = servername;
        }
      }

    } else if (strcasecmp(parts[0].c_str(), "DocumentRoot") == 0) {
      if (currentvhost != NULL && parts.size() == 2) {
        currentvhost->documentroot = parts[1];
      }

    } else if (strcasecmp(parts[0].c_str(), "<VirtualHost") == 0) {
      if (line[line.length() - 1] != '>') continue;
      vhosts_.push_back(VHostConfig());
      currentvhost = &vhosts_.back();  // dangerous code?
      currentvhost->address =
          line.substr(parts[0].length(), line.length() - parts[0].length() - 1);

    } else if (strcasecmp(parts[0].c_str(), "</VirtualHost>") == 0) {
      currentvhost = &mainserver_;

    } else if (strcasecmp(parts[0].c_str(), "Include") == 0) {
      if (parts.size() != 2) continue;

      std::vector<std::string> files;
      if (!RetrieveFiles(parts[1].c_str(), &files)) continue;

      for (int i = 0; i < static_cast<int>(files.size()); ++i) {
        ProcessFile(files[i].c_str());
      }

    } else if (strcasecmp(parts[0].c_str(), "Group") == 0) {
      if (parts.size() != 2) continue;
      group_name_ = parts[1];

    } else if (strcasecmp(parts[0].c_str(), "CustomLog") == 0) {
      if (currentvhost != NULL && parts.size() == 3) {
        currentvhost->customlog = parts[1];
      }
    }
  }

  return true;
}

// Whether the pattern contains glob definition according to POSIX fnmatch
// method.
static bool IsFnmatchPattern(const char* pattern) {
  bool nesting = false;
  for (; *pattern != '\0'; ++pattern) {
    switch (*pattern) {
      // *, ? glob
      case '?':
      case '*':
        return true;

      // [] glob
      case '[':
        nesting = true;
        break;
      case ']':
        if (nesting) return true;
        break;

      // escape chars
      case '\\':
        ++pattern;
        if (*pattern == '\0') {
          return false;
        }
        break;
    }
  }

  return false;
}

// Check whether given path repsents a dir, exclude symbol link.
static bool IsRealDirectory(const char* path) {
  if (path == NULL) return false;

  struct stat dirinfo;
  if (lstat(path, &dirinfo) != 0) {
    return false;
  }

  return (S_ISDIR(dirinfo.st_mode)) &&
     (!(S_ISLNK(dirinfo.st_mode)));
}

// retrieve files for a given fname, which can be a regex pattern or a file name
bool ApacheConfig::RetrieveFiles(const char* fname,
                                 std::vector<std::string>* files) {
  std::string filename(fname);
  if (!MakeAbsolutePath(&filename)) {
    return false;
  }

  // a regular file with no regex in name
  if (!IsFnmatchPattern(filename.c_str())
      && !IsRealDirectory(filename.c_str())) {
    files->push_back(filename.c_str());
    return true;
  }

  std::string dirname, pattern;
  if (IsRealDirectory(filename.c_str())) {
    dirname = filename;
  } else {
    // fname should be an absolute path
    size_t pos = filename.rfind('/');
    if (pos == std::string::npos) return false;

    // separate dir and file name pattern
    dirname = filename.substr(0, pos);
    pattern = filename.substr(pos + 1);
  }

  DIR* dir = opendir(dirname.c_str());
  if (dir == NULL) return false;

  // iterate the dir to match file pattern
  struct dirent* entry = readdir(dir);
  while (entry != NULL) {
    if (entry->d_name != NULL &&
        strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      if (pattern.length() == 0 ||
          fnmatch(pattern.c_str(), entry->d_name, FNM_PERIOD)  == 0) {
        std::string fullpath(dirname);
        fullpath.append("/").append(entry->d_name);
        if (IsRealDirectory(fullpath.c_str())) {
          RetrieveFiles(fullpath.c_str(), files);
        } else {
          files->push_back(fullpath);
        }
      }
    }

    entry = readdir(dir);
  }
  closedir(dir);

  return true;
}

// build site id for a vhost from a vhost config
// This method mimics the way by which apache resolve server names.
// The idea comes from $get_addresses and $ap_fini_vhost_config of apache main
// source.
bool ApacheConfig::BuildSiteId(VHostConfig* conf, std::string* siteid,
                               int defaultport) {
  std::vector<std::string> addrstrs = Split(conf->address);

  // we can do nothing for a virtualhost with no address or name.
  if (addrstrs.size() == 0 && conf->servername.length() == 0) {
    return false;
  }

  // <VirtualHost name1:port1 name2:port2...>
  // NOTE, if vector is used, it must be sorted.
  std::set<std::pair<std::string, int> > vhostaddrs;

  for (int i = 0, isize = static_cast<int>(addrstrs.size());
       i < isize; ++i) {
    int addrlen = static_cast<int>(addrstrs[i].length());
    std::string addrstr = addrstrs[i];

    // some special handling
    if (addrlen == 0) continue;
    if (addrstr == ":*") {
      // For main server.
      vhostaddrs.insert(std::make_pair("", 0));
      continue;
    }

    int port = defaultport;
    size_t pos = addrstr.rfind(':');
    std::string host_str;
    if (pos == std::string::npos) {
      host_str = addrstr;
    } else {
      host_str = addrstr.substr(0, pos);

      std::string port_str = addrstr.substr(pos + 1);
      int tmpport = 0;
      if (port_str == "*") {
        port = 0;
      } else if ((tmpport = atoi(port_str.c_str())) != 0) {
        port = tmpport;
      } else {
        Logger::Log(EVENT_ERROR, "Invalid port: [%s].", port_str.c_str());
        return false;
      }
    }

    vhostaddrs.insert(make_pair(host_str, port));
  }

  // someone is silly to forget the servername, so:
  if (conf->servername.length() == 0) {
    // if it contains default or wildcard value:
    std::set<std::pair<std::string, int> >::iterator itr = vhostaddrs.begin();
    for (; itr != vhostaddrs.end(); ++itr) {
      std::string host = itr->first;
      if (strcmp(host.c_str(), "*") == 0 ||
          strcasecmp(host.c_str(), "_default_") == 0) {
        conf->servername = mainserver_.servername;
        break;
      }
    }

    // else, Random pick a servername 
    if (conf->servername.length() == 0) {
      // 1) Resolve address a host string.
      std::string host = vhostaddrs.begin()->first;
      struct hostent* host_entry = gethostbyname(host.c_str());
      if (host_entry == NULL || host_entry->h_addrtype != AF_INET
          || host_entry->h_addr_list[0] == NULL) {
        Logger::Log(EVENT_ERROR, "Can't resolve host name [%s].", host.c_str());
        return false;
      }

      // 2) do reverse DNS to get server name.
      host_entry = gethostbyaddr(
          reinterpret_cast<char*>(host_entry->h_addr_list[0]),
          sizeof(struct in_addr), AF_INET);
      if (host_entry != NULL && host_entry->h_name != NULL) {
        conf->servername = host_entry->h_name;
      } else {
        conf->servername = "bogus_host_without_reverse_dns";
      }
    }
  }

  *siteid = conf->servername;

  std::set<std::pair<std::string, int> >::iterator itr = vhostaddrs.begin();
  for (; itr != vhostaddrs.end(); ++itr) {
    siteid->append(",").append(itr->first);

    // append the port to siteid
    char buffer[128];
    itoa(itr->second, buffer);
    siteid->append(":").append(buffer);
  }

  return true;
}

bool ApacheConfig::NormalizeServerName(std::string* servername) {
  // strip scheme
  std::string::size_type pos = servername->find("://");
  if (pos != std::string::npos) {
    *servername = servername->substr(pos + strlen("://"));
  }

  // strip port
  pos = servername->find(":");
  if (pos != std::string::npos) {
    *servername = servername->substr(0, pos);
  }

  return servername->length() != 0;
}
