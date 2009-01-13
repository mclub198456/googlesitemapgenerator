#!/bin/sh
#
# Copyright 2009 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e

PKG_NAME="google-sitemap-generator"

BIN_DIR="bin"
LIB_DIR="lib"
RUN_DIR="run"
LOG_DIR="log"
RES_DIR="resource"
CONF_DIR="conf"
REPO_DIR="cache"

MODULE_LIB="mod_sitemap.so"
MODULE_NAME="google_sitemap_generator_module"
DAEMON_BIN="sitemap-daemon"
DAEMON_LINK="/usr/sbin/google-sitemap-generator-ctl"

PID_FILE="$PKG_NAME.pid"
APP_DIR="./"

GLOBAL_CONF_DIR="/etc/google-sitemap-generator"
GLOBAL_CONF_HOME="home"

APACHE_INFO="apache.info"

APACHE_SCRIPT="apache.sh"
UNINSTALL_SCRIPT="uninstall.sh"

APACHE="_APACHE_BIN_"

###########################################################
# Execute a command.
# $1 error handling flag. 0 = exit. 1 = ignore
# $2... command with args
ExecCmd()
{
  local error_flag=$1
  shift

  if $@; then :; else
    if [ $error_flag -eq 0 ]; then
      echo "Failed to execute command: $@"
      exit 1;
    else
      echo "WARNING: Failed to execute command: $@, ignore."
      return 0
    fi
  fi
}

GetAppDir()
{
  APP_DIR=`dirname "$0"`
  if [ -z "$APP_DIR" ]; then
    APP_DIR="."
  fi

  if [ "$APP_DIR" = "." ]; then
    APP_DIR=`pwd`
  fi

  if [ "$APP_DIR" = ".." ]; then
    APP_DIR=`pwd`
    APP_DIR=`dirname "$APP_DIR"`
  fi

  return 0
}

CheckApache()
{
#  if test "x$APACHE" = "x_APACHE_BIN_"; then
#    echo "This uninstall script is only a template."
#    exit 1
#  fi

  if test -f "$APP_DIR/$APACHE_INFO"; then :; else
    echo "Failed to find apache.info in installation directory."
    exit 1
  fi

  if LoadApacheInfo "$APP_DIR/$APACHE_INFO"; then :; else
    echo "Failed to load apache.info from installation directory."
    exit 1
  fi
}

# Simply try to stop it.
StopSitemapDaemon()
{
  # stop sitemap service
  if [ -f "$APP_DIR/$RUN_DIR/$PID_FILE" ]; then
    echo "The Google Sitemap Generator daemon is running. Stopping the daemon..."
    if "$APP_DIR/$BIN_DIR/$DAEMON_BIN" service stop 1>/dev/null 2>/dev/null; then
      echo "Google Sitemap Generator daemon successfully stopped."
    else
      echo "Failed to stop Sitemap daemon."
      exit 1
    fi
  fi

  return 0
}

CleanRobotsTxt()
{
  if "$APP_DIR/$BIN_DIR/$DAEMON_BIN" clean_robots 1>/dev/null 2>/dev/null; then
    echo "Sitemap lines successfully removed from robots.txt."
  else
    echo "Failed to remove Sitemap lines from robots.txt."
  fi

  return 0
}

# uninstall system service
UninstallInitScript_Linux()
{
  local rc_parent=""
  if test -d "/etc/rc0.d"; then
    rc_parent="/etc"
  elif test -d "/etc/init.d/rc0.d"; then
    rc_parent="/etc/init.d"
  elif test -d "/etc/rc.d/rc0.d"; then
    rc_parent="/etc/rc.d"
  else
    return 0
  fi

  INITD_FILE="$PKG_NAME"

  ExecCmd 0 rm -f $rc_parent/rc2.d/S90$INITD_FILE
  ExecCmd 0 rm -f $rc_parent/rc3.d/S90$INITD_FILE
  ExecCmd 0 rm -f $rc_parent/rc4.d/S90$INITD_FILE
  ExecCmd 0 rm -f $rc_parent/rc5.d/S90$INITD_FILE

  ExecCmd 0 rm -f $rc_parent/rc0.d/K10$INITD_FILE
  ExecCmd 0 rm -f $rc_parent/rc1.d/K10$INITD_FILE
  ExecCmd 0 rm -f $rc_parent/rc2.d/K10$INITD_FILE
  ExecCmd 0 rm -f $rc_parent/rc3.d/K10$INITD_FILE
  ExecCmd 0 rm -f $rc_parent/rc4.d/K10$INITD_FILE
  ExecCmd 0 rm -f $rc_parent/rc5.d/K10$INITD_FILE
  ExecCmd 0 rm -f $rc_parent/rc6.d/K10$INITD_FILE

  ExecCmd 0 rm -f /etc/init.d/$INITD_FILE

  return 0
}

UninstallInitScript_FreeBSD()
{
  local conf_file="/etc/rc.local"
  local new_conf=`mktemp /tmp/gsg_tmp.XXXXXX`
  grep -v google-sitemap-generator "$conf_file" > $new_conf || echo > $new_conf
  ExecCmd 0 mv -f $new_conf "$conf_file"

  return 0
}

UninstallInitScript()
{
  local os_name=`uname`
  if [ "x$os_name" = "xLinux" ]; then
    if UninstallInitScript_Linux; then :; else
      return 1
    fi
  elif [ "x$os_name" = "xFreeBSD" ]; then
    if UninstallInitScript_FreeBSD; then :; else
      return 1
    fi
  else
    return 0
  fi

  echo "Daemon auto-start scripts successfully removed."
  return 0
}

# Modify Apache Conf File.
ModifyApacheConf()
{
  # deactivate sitemap module in configuration
  local new_conf=`mktemp /tmp/gsg_tmp.XXXXXX`
  ExecCmd 0 cp -f "$APACHE_CONF" $GLOBAL_CONF_DIR/httpd.uninstall.conf
  ExecCmd 0 grep -v "google-sitemap-generator" "$APACHE_CONF" > $new_conf
  ExecCmd 0 mv -f $new_conf "$APACHE_CONF"
  echo "Apache configuration successfully updated."
  echo "Old configuration saved at $GLOBAL_CONF_DIR/httpd.uninstall.conf"

  return 0
}

UninstallProgramFiles()
{
  ExecCmd 0 rm -f "$DAEMON_LINK"
  ExecCmd 0 rm -f "$APP_DIR/$BIN_DIR/$DAEMON_BIN"
  ExecCmd 0 rm -rf "$APP_DIR/$LIB_DIR"
  ExecCmd 0 rm -rf "$APP_DIR/$RUN_DIR"
  ExecCmd 0 rm -rf "$APP_DIR/$RES_DIR"

  ExecCmd 0 rm -f "$GLOBAL_CONF_DIR/$GLOBAL_CONF_HOME"

  echo "Program files successfully removed."
  echo ""
  return 0
}

PostUninstall()
{
  # Remove data base files.
  cat <<HERE_POST_UNINSTALL
The URL database and application settings file can be saved and used when you
reinstall. If you choose to save the files now, but later you want to remove
them, you can run this script again.
HERE_POST_UNINSTALL
echo -n "Do you want to save the URL database and application settings file ? [Y/n]"
  read answer
  echo ""
  if [ "$answer" = "n" -o "$answer" = "N" ]; then
    ExecCmd 0 rm -rf "$APP_DIR"
    echo "URL database and application settings successfully cleaned."
  else
    echo "You can run this script again to remove those files."
  fi

  echo "Google Sitemap Generator module still works until Apache is restarted."
  if [ "x$APACHE_CTL" = "x" ]; then
    echo "You should restart Apache manually to disable the Google Sitemap Generator module."
  else
    echo -n "Do you want to restart Apache now? [N/y]"
    read answer
    echo ""
    if [ "$answer" = "Y" -o "$answer" = "y" ]; then
      if RestartApache; then :; else
        echo "Failed to restart Apache. You need to restart it manually."
      fi
    fi
  fi

  echo "Google Sitemap Generator (Beta) successfully uninstalled."
  return 0
}

# ---------------------------------------------------------
# ensure user is root
if [ `id -u` != "0" ]; then
  echo "Permission denied. Uninstallation script must be run with superuser privileges."
  exit 1
fi

GetAppDir

# import apache contorl script
if test -f "$APP_DIR/$BIN_DIR/apache.sh" ; then :; else
  echo "($APP_DIR/$BIN_DIR/apache.sh) can't be found."
  exit 1
fi
. "$APP_DIR/$BIN_DIR/apache.sh"

CheckApache

ModifyApacheConf

UninstallInitScript

StopSitemapDaemon

CleanRobotsTxt

UninstallProgramFiles

PostUninstall

exit 0
