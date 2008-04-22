#!/bin/bash
#
# Copyright 2008 Google Inc.
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


# ensure user is root
if [ $(id -u) != 0 ]; then
  echo "Permission denied: require superuser"
  exit 1
fi


DIR=`dirname "$0"`
if test -f "$DIR/apache.sh" ; then :; else
  echo "($DRI/apache.sh) can't be found"
  exit 1
fi
. "$DIR/apache.sh"

APP_DIR=/usr/share/google-sitemap-generator
DAEMON_FILE=sitemap-daemon
MODULE_FILE=mod_sitemap.so
MODULE_NAME=google_sitemap_generator_module
CTL_FILE=google-sitemap-generator-ctl
PID_FILE=google-sitemap-generator.pid
INITD_FILE=google-sitemap-generator

CONF_DIR=/etc/google-sitemap-generator
DATA_DIR=/var/spool/google-sitemap-generator
LOCK_DIR=/var/lock/google-sitemap-generator

APACHE="_APACHE_BIN_"

###########################################################

# Pick a file from a list of files or user input.
# Please be careful when the file path contains whitespaces.
#  $1  The name of variable used to store final result
#  $2  File testing flag, "-f" for file, "-d" for directory
#  $3  Flag indicating whether always requires user input
#  $4  Message promoted for user
#  $5  A list of choices for user
function PickFile()
{
  # Declare more meaningful names for args
  local result=""
  local flag=$2
  local requireinput=$3
  local msg=$4
  local choice=$5

  # Pick first existing file from choice list
  for file in $choice
  do
    if [ $flag $file ]; then
      result=$file
      break
    fi
  done

  if [ -n "$file" -a "$requireinput" = "0" ]; then
    eval "$1=$result"
    return 0
  fi

  # Get a existing file from user
  # the default one is from choice list
  while [ 0 ]
  do
    echo -n "$msg[$result]"
    read answer
    if [ -z "$answer" ]; then
      answer=$result
    fi
    if [ $flag "$answer" ]; then
      result=$answer
      break;
    fi
  done

  # success. Save the result.
  echo "[Info] Ok.[$result] will be used."
  eval "$1=$result"
  return 0
}

# Execute a command.
# $1 error handling flag. 0 = exit. 1 = ignore
# $2... command with args
function ExecCmd()
{
  local error_flag=$1
  shift
  "$@"

  if [ $? -ne 0 ]; then
    echo "Failed to execute command: $@"
    if [ $error_flag -e 0 ]; then
      echo "[Failed] Can't execute command: $@"
      exit 1;
    else
      echo "[Warn] Can't execute command: $@, ignore."
    fi
  fi
}

function PreUninstall()
{
  if test "x$APACHE" = "x_APACHE_BIN_"; then
    PickFile APACHE -f 1 "Which is apache bin?" \
    "/usr/sbin/apache2 /usr/sbin/apache /usr/sbin/httpd /usr/local/httpd/bin/httpd"
  fi

  AnalyzeApache "$APACHE"
  if test $? -ne 0 ; then
    echo "($APACHE) is not a supported apache binary instance."
    exit 1
  fi
}

# uninstall sitemap service
# Simply try to stop it.
function UninstallSitemapService()
{
  # stop sitemap service
  # if [ $(ps axu | grep $SERVICE | grep -v "grep" | wc -l) -gt 0 ]; then
  if [ -f "/var/run/$PID_FILE" ]; then
    echo "[Warn] Maybe google sitemap generator is running."
    echo -n "[Question] Do you want to stop it? [Y/n]"
    read answer
    if [ "$answer" = "N" -o "$answer" = "n" ]; then
      echo "[Failed] Please uninstall it next time."
      exit 1
    else
      /usr/sbin/$CTL_FILE -stop
      if [ $? != 0 ]; then
        echo "[Failed] Please stop is manually."
        exit 1
      fi
    fi
  fi

  return 0
}

# uninstall system service
function UninstallAutoStartScript()
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

  echo "[Done] System service is deleted."
  return 0
}

# uninstall sitemap apache2 module
function UninstallSitemapModule()
{
  # Stop the running apache first.
  ApacheStatus

  if test $APACHE_RUNNING -eq 1; then
    echo "[Warn] Apache is running and sitemap module is loaded."
    echo -n "[Question] Do you want to stop apache server?[Y/n]"
    read answer
    if [ "$answer" = N -o "$answer" = n ]; then
      echo "[Failed] Please uninstall later, bye!"
      exit 1
    else
      StopApache
      if test $? -ne 0; then
        echo "Failed to stop apache."
        exit 1
      fi
    fi
  fi

  # deactivate sitemap module in configuration
  local new_conf=`mktemp`
  ExecCmd 0 cp -f "$APACHE_CONF" $APP_DIR/httpd.uninstall.conf
  ExecCmd 0 grep -v mod_sitemap.so "$APACHE_CONF" > $new_conf
  ExecCmd 0 cp -f $new_conf "$APACHE_CONF"
  echo "[Info] The old configuration is kept in $APP_DIR/httpd.uninstall.conf"

  echo "[Done] Sitemap module is removed from apache configuration."
  return 0
}

function UninstallProgramFiles()
{
  ExecCmd 0 rm -f /usr/sbin/$CTL_FILE
  ExecCmd 0 rm -f $APP_DIR/restart.sh
  ExecCmd 0 rm -f $APP_DIR/$MODULE_FILE
  ExecCmd 0 rm -f $APP_DIR/$DAEMON_FILE
}

function PostUninstall()
{
  echo "[Info] Do you want to remove all the files which could be used later?"
  echo "[Info] It is strongly recommended to keep them."
  echo -n "[Info] If NO, You could rerun this script to remove all of them.[N/y]."

  read answer
  if [ "$answer" = "Y" -o "$answer" = "y" ]; then
    rm -rf $CONF_DIR
    rm -rf $DATA_DIR
    rm -rf $APP_DIR
    rm -rf $LOCK_DIR
    echo "All files are cleaned."
  fi
  return 0
}
# main uninstall procedure
ERROR=0

PreUninstall

UninstallSitemapModule

UninstallAutoStartScript

UninstallSitemapService

UninstallProgramFiles

PostUninstall

if [ $ERROR -eq "1" ]; then
  echo "[All Done] Uninstall is successful."
fi
exit 0
