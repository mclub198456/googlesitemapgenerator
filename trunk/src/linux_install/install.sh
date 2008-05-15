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
SRC_DIR=$DIR

# import apache contorl script
if test -f "$SRC_DIR/apache.sh" ; then :; else
  echo "($SRC_DRI/apache.sh) can't be found"
  exit 1
fi
. "$SRC_DIR/apache.sh"

MODULE_FILE="mod_sitemap.so"
MODULE_NAME="google_sitemap_generator_module"
DAEMON_FILE="sitemap-daemon"

APP_DIR="/usr/share/google-sitemap-generator"
DATA_DIR="/var/spool/google-sitemap-generator"
LOCK_DIR="/var/lock/google-sitemap-generator"
CONF_DIR="/etc/google-sitemap-generator"

LOG_FILE="google-sitemap-generator.log"
CONF_FILE="sitesettings.xml"
CTL_FILE="google-sitemap-generator-ctl"
INITD_FILE="google-sitemap-generator"

APACHE=""

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
    if [ $error_flag -eq 0 ]; then
      echo "[Failed] Can't execute command: $@"
      exit 1;
    else
      echo "[Warn] Can't execute command: $@, ignore."
    fi
  fi
}

function ShowLicense()
{
ExecCmd 0 more -d "$SRC_DIR/tos"
echo -n "Do you agree the terms of service? [N/y]"
read answer
if [ "x$answer" != "xy" -a "x$answer" != "xY" ]; then
  echo "Exiting..."
  exit 1
fi

cat <<WELCOME_MSG
**************************************************************************
        Welcome to Google Sitemap Generator (Beta)!

For more information, please visit:
  http://code.google.com/p/googlesitemapgenerator/
**************************************************************************
WELCOME_MSG
}

# Get options from command line.
function GetOptions()
{
  for param in "$@"
  do
    key=`echo "$param" | awk -F= '{ print $1 }'`
    value=`echo "$param" | awk -F= '{ print $2 }'`

    case "x$key" in
      "x--apache_bin")
      APACHE="$value"
      ;;
      *)
      echo "Unknown option: [$key]"
      exit 1
      ;;
    esac
  done

  if test "x$APACHE" = "x"; then
    PickFile APACHE -f 1 "Where is apache bin?" \
    "/usr/sbin/apache2 /usr/sbin/apache /usr/sbin/httpd /usr/local/httpd/bin/httpd"
  fi

  AnalyzeApache "$APACHE"
  if test $? -ne 0 ; then
    echo "($APACHE) is not a supported apache binary instance."
    exit 1
  fi

  return 0
}

# do some preinstall checking
function PreInstall()
{
  # ensure all permission flags could set properly.
  umask 0

  # check previous version
  if [ -f "$APP_DIR/$MODULE_FILE" -o -f "$APP_DIR/$DAEMON_FILE" ]; then
    echo "[Info] It seems that this application is already installed."
    echo -n "[Question] Do you want to uninstall it first?[Y/n]"
    read answer
    if [ "$answer" = "N" -o "$answer" = "n" ]; then
      echo "[Warn] It is strongly recommened to uninstall it first."
    else
      $APP_DIR/uninstall.sh
    fi
  fi

cat <<HERE_APACHE_MESSAGE
*********************************************************************
From your apache binary, we have detected that,
  1) Apache version is: [Version $APACHE_VERSION]
  2) Apache architecture is: [$APACHE_ARCH bits]
  3) Apache root configuration file is: [$APACHE_CONF]
  4) Apache pid file is: [$APACHE_PIDFILE]

If you find any thing above is incorrect, please DO NOT continue.
*********************************************************************
HERE_APACHE_MESSAGE
echo -n "Do you want to proceed?[N/y]"
read answer
if [ "x$answer" != "xy" -a "x$answer" != "xY" ]; then
  echo "Sorry! Exiting..."
  exit 1
fi

cat <<HERE_START_MESSAGE
*********************************************************************
This application assumes that,
    1) Apache httpd is installed, and DSO is enabled.
Following directories will be used by this application:
    1) /var/spool/google-sitemap-generator
    2) /usr/share/google-sitemap-generator
    3) /etc/google-sitemap-generator
    4) /var/lock/google-sitemap-generator
Following files will be used by this application:
    1) /usr/sbin/google-sitemap-generator-ctl
    2) /var/log/google-sitemap-generator.log
    3) /var/run/google-sitemap-generator.pid
You could remove all application files by running following command.
    sudo /usr/share/google-sitemap-generator/uninstall.sh
*********************************************************************
HERE_START_MESSAGE

echo -n "Do you want to proceed?[Y/n]"
read answer
if [ "$answer" = "n" -o "$answer" = "N" ]; then
  echo "Sorry! Exiting..."
  exit 1
fi

# prepare module file
local mod_name=mod_sitemap.$APACHE_VERSION.$APACHE_ARCH.so
if test -f "$SRC_DIR/$mod_name"; then :; else
  echo "$mod_name is missing."
  exit 1
fi

ExecCmd 0 rm -f "$SRC_DIR/mod_sitemap.so"
ExecCmd 0 cp "$SRC_DIR/$mod_name" "$SRC_DIR/mod_sitemap.so"

# prepare daemon binary file.
local daemon_name=sitemap-daemon.$APACHE_ARCH
if test -f "$SRC_DIR/$daemon_name"; then :; else
  echo "$daemon_name is missing."
  exit 1
fi

ExecCmd 0 rm -f "$SRC_DIR/sitemap-daemon"
ExecCmd 0 cp "$SRC_DIR/$daemon_name" "$SRC_DIR/sitemap-daemon"

return 0;
}

# Install program files.
function InstallProgramFiles()
{
echo "[Info] Install program files..."

# make dir for the application to use.
ExecCmd 0 mkdir -p $CONF_DIR
ExecCmd 0 mkdir -p $APP_DIR
ExecCmd 0 mkdir -p $DATA_DIR
ExecCmd 0 mkdir -p $LOCK_DIR

# change permission flags for all the dirs.
ExecCmd 0 chmod 0755 $CONF_DIR
ExecCmd 0 chmod 0755 $APP_DIR
ExecCmd 0 chmod 0755 $DATA_DIR
ExecCmd 0 chmod 0755 $LOCK_DIR

# Save apache conf to a file for uninstall.
ExecCmd 0 echo $APACHE_CONF > $APP_DIR/.apachebin
ExecCmd 0 chmod 644 "$APP_DIR/.apachebin"

# generate restart script
local temp_restart_sh=`mktemp`
local escaped_bin=`echo $APACHE_BIN | sed 's/\//\\\\\//g'`
ExecCmd 0 sed "s/\"_APACHE_BIN_\"/$escaped_bin/" \
  $SRC_DIR/restart.sh > $temp_restart_sh

# generate uninstall script
local temp_uninstall_sh=`mktemp`
local escaped_bin=`echo $APACHE_BIN | sed 's/\//\\\\\//g'`
ExecCmd 0 sed "s/\"_APACHE_BIN_\"/$escaped_bin/" \
  $SRC_DIR/uninstall.sh > $temp_uninstall_sh


# Copy files
ExecCmd 0 cp -f $temp_uninstall_sh $APP_DIR/uninstall.sh
ExecCmd 0 cp -f "$SRC_DIR/$DAEMON_FILE" $APP_DIR
ExecCmd 0 cp -f "$SRC_DIR/$CTL_FILE" /usr/sbin/
ExecCmd 0 cp -f $temp_restart_sh $APP_DIR/restart.sh
ExecCmd 0 cp -rf "$SRC_DIR/templates" $APP_DIR
ExecCmd 0 cp -f "$SRC_DIR/apache.sh" $APP_DIR

# Copy module file according to whether SELinux is enabled.
if which selinuxenabled &> /dev/null; then
  local context="system_u:object_r:httpd_modules_t"
  echo "[Info] SELinux is enabled."
  echo "[Info] Change security context of $MODULE_FILE to $context"
  ExecCmd 0 install -Z $context "$SRC_DIR/$MODULE_FILE" $APP_DIR
else
  ExecCmd 0 cp -f "$SRC_DIR/$MODULE_FILE" $APP_DIR
fi

# Change file permissions
ExecCmd 0 chmod 0750 "$APP_DIR/uninstall.sh"
ExecCmd 0 chmod 0750 "$APP_DIR/apache.sh"
ExecCmd 0 chmod 0750 "$APP_DIR/$DAEMON_FILE"
ExecCmd 0 chmod 0750 "$APP_DIR/restart.sh"
ExecCmd 0 chmod 0755 "$APP_DIR/$MODULE_FILE"
ExecCmd 0 chmod 0755 /usr/sbin/$CTL_FILE

ExecCmd 0 find "$APP_DIR/templates" -type d -exec chmod 755 {} \;
ExecCmd 0 find "$APP_DIR/templates" -type f -exec chmod 644 {} \;

echo "[Done] Program files are installed properly."
return 0;
}

# update site settings
function UpdateSiteSettings()
{
  echo "[Info] Update site settings..."

  # $APPDIR/$SERVICE update_setting "httpdconf=$APACHECONF"
  /usr/sbin/$CTL_FILE -update_setting "apache_conf=$APACHE_CONF"

  if [ $? ]; then
    echo "[Done] Sitesettings are updated properly."
    return 0
  else
    echo "[Failed] Can't update sitesettings."
    return 1
  fi
}

# Install auto start to script to auto start sitemapservice automatically
function InstallAutoStartScript()
{
  echo "[Info] Start to install sitemapservice as an auto-start daemon..."

  if test -d "/etc/init.d"; then :; else
    echo "[Warn] No such directory: (/etc/init.d)"
    echo "[Warn] Auto-start script is not installe.d"
    return 0
  fi

  local service_file="/etc/init.d/$INITD_FILE"

  # check whether a same name autostart script exist
  if [ -f "/etc/init.d/$INITD_FILE" ]; then
    echo "[Warn] There is already a system service named as [$INITD_FILE]"
    echo -n "[Warn] Do you REALLY want to proceed any way?[N/y]"
    read answer
    if [ "$answer" = N -o "$answer" = n -o -z "$answer" ]; then
      return 1
    else
      echo "[Warn] The old $INITD_FILE will be overwritten."
    fi
  fi

  # retrieve current run level
  # RUNLEVEL=$(runlevel | awk '{print $2}')
  # echo "[Warn] Auto start service will be installed at level $RUNLEVEL"

  # copy the start script to /etc/init.d
  ExecCmd 0 cp -f $SRC_DIR/autostart.sh $service_file
  ExecCmd 0 chmod 0755 $service_file

  # Make link for start scripts
  local rc_parent=""
  if test -d "/etc/rc0.d"; then
    rc_parent="/etc"
  elif test -d "/etc/init.d/rc0.d"; then
    rc_parent="/etc/init.d"
  elif test -d "/etc/rc.d/rc0.d"; then
    rc_parent="/etc/rc.d"
  else
    echo "[Warn] System startup scripts link dir can't be found."
    return 0
  fi

  ExecCmd 1 ln -fs $service_file $rc_parent/rc0.d/K10$INITD_FILE
  ExecCmd 1 ln -fs $service_file $rc_parent/rc1.d/K10$INITD_FILE
  ExecCmd 1 ln -fs $service_file $rc_parent/rc2.d/K10$INITD_FILE
  ExecCmd 1 ln -fs $service_file $rc_parent/rc3.d/K10$INITD_FILE
  ExecCmd 1 ln -fs $service_file $rc_parent/rc4.d/K10$INITD_FILE
  ExecCmd 1 ln -fs $service_file $rc_parent/rc5.d/K10$INITD_FILE
  ExecCmd 1 ln -fs $service_file $rc_parent/rc6.d/K10$INITD_FILE

  ExecCmd 1 ln -fs $service_file $rc_parent/rc2.d/S90$INITD_FILE
  ExecCmd 1 ln -fs $service_file $rc_parent/rc3.d/S90$INITD_FILE
  ExecCmd 1 ln -fs $service_file $rc_parent/rc4.d/S90$INITD_FILE
  ExecCmd 1 ln -fs $service_file $rc_parent/rc5.d/S90$INITD_FILE

  echo "[Done] $INITD_FILE is intalled as a system service"
  return 0
}

# install sitemap apache module
function InstallApacheModule()
{
  echo "[Info] Start to update apache conf..."

  # update the configuration file
  local temp_conf=`mktemp`
  ExecCmd 0 cp -f $APACHE_CONF $APP_DIR/httpd.install.conf
  ExecCmd 0 grep -v "$APP_DIR/$MODULE_FILE" $APACHE_CONF > $temp_conf
  ExecCmd 0 echo "LoadModule $MODULE_NAME $APP_DIR/$MODULE_FILE" >> $temp_conf
  ExecCmd 0 cp -f $temp_conf "$APACHE_CONF"

  echo "[Info] The old conf is kept as $APP_DIR/httpd.install.conf"

  # apxs or apxs2 may be not installed...
  # apxs2 -i -n sitemap -a $SRCDIR/$MODULE
  # if [ $? != 0 ]; then
  #   echo "[Failed] Can't install sitemap apache module. Exiting..."
  #   return 1
  # fi

  echo "[Done] $MODULE_NAME module is installed."

  return 0
}

# try to start service and apache2
function StartServices()
{
  echo -n "[Question] Do you want to start sitemap generator immediately?[N/y]"
  read answer
  if [ "$answer" = Y -o "$answer" = y ]; then
    /usr/sbin/$CTL_FILE -start
  fi

  echo -n "[Question] Do you want to (re)start apache server immediately?[N/y]"
  read answer
  if [ "$answer" = Y -o "$answer" = y ]; then
    $APACHE_BIN $APACHE_ARG restart
  fi
  return 0
}

function PostInstall()
{
  /usr/sbin/$CTL_FILE -start
  if [ $? != 0 ]; then
    echo "[Failed] Sitemap generator daemon can't be started."
    return 1
  fi

  PickFile browser -f  1 "Which browser should be used to configure?" \
  "/usr/bin/firefox /usr/bin/mozilla /usr/bin/konqueror /usr/bin/opera"

  $browser http://localhost:8181/ &
  return 0
}

# main procedure

ShowLicense

GetOptions

PreInstall

InstallProgramFiles

UpdateSiteSettings

InstallAutoStartScript

InstallApacheModule

PostInstall

echo "[All Done] You can uninstall it with /usr/share/google-sitemap-generator/uninstall.sh"
exit 0
