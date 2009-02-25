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
CONF_DIR="conf"
REPO_DIR="cache"
ADM_DIR="admin-console"
CGI_DIR="admin-console/cgi-bin"

PREFIX="/usr/local"

DAEMON_BIN="sitemap-daemon"
ADMIN_BIN="admin-console-cgi"
MODULE_LIB="mod_sitemap.so"

MODULE_NAME="google_sitemap_generator_module"
DAEMON_LINK="/usr/sbin/google-sitemap-generator-ctl"
CGI_BIN="admin-console.cgi"

HTTPD_CONF="httpd.conf"

DEST_DIR="$PREFIX/$PKG_NAME"
SRC_DIR="./"

APACHE_INFO="apache.info"
GLOBAL_CONF_DIR="/etc/google-sitemap-generator"
GLOBAL_CONF_HOME="home"

APACHE_SCRIPT="apache.sh"
UNINSTALL_SCRIPT="uninstall.sh"
TOS_FILE="tos"
WARN_FILE="privacy_warning"
REMOTE_ADMIN="false"
AUTO_SUBMISSION="disable"

arg_apache_binary=""
arg_apache_group=""
arg_apache_conf=""
arg_apache_ctl=""

###########################################################

GetSrcDir()
{
  SRC_DIR=`dirname "$0"`
  if [ -z "$SRC_DIR" ]; then
    SRC_DIR="./"
  fi

  return 0
}

InitDestDir()
{
  echo "Google Sitemap Generator will be installed into"
  echo "$DEST_DIR"
  echo ""

  if [ -d "$DEST_DIR" ]; then
    echo "Directory ($DEST_DIR) already exists."
    echo "Do you want to install Google Sitemap Generator into this existing directory,"
    echo -n "overwriting old files? [N/y]"
    read answer
    if [ "$answer" = "Y" -o "$answer" = "y" ]; then :; else
      echo "The Google Sitemap Generator installation has been aborted."
      exit 1
    fi
  fi

  ExecCmd 0 mkdir -p "$DEST_DIR"
  return 0
}

CheckOldInstallation()
{
  local home_file="$GLOBAL_CONF_DIR/$GLOBAL_CONF_HOME"
  if [ -f "$home_file" ]; then :; else
    return 0
  fi

  local old_home=`ExecCmd 0 grep "$PKG_NAME" "$home_file"`
  echo "There is already an existing installation of Google Sitemap Generator in"
  echo "$old_home."
  echo -n "Do you want to uninstall the existing version first? [Y/n]"
  read answer
  echo ""
  if [ "x$answer" = "xN" -o "x$answer" = "xn" ]; then
    echo "You must uninstall the existing version before proceeding with this \
installation."
    echo -n "Do you want to cancel this installation? [Y/n]"
    read answer
    if [ "x$answer" = "xN" -o "x$answer" = "xn" ]; then :; else
      echo "The Google Sitemap Generator installation has been aborted."
      exit 1
    fi
  fi

  ExecCmd 0 "$old_home/$UNINSTALL_SCRIPT"
  echo -n "Press ENTER to continue..."
  read answer
  echo ""

  return 0
}


# Pick a file from a list of files or user input.
# Please be careful when the file path contains whitespaces.
#  $1  The name of variable used to store final result
#  $2  File testing flag, "-f" for file, "-d" for directory
#  $3  Flag indicating whether always requires user input
#  $4  Message promoted for user
#  $5 $6 ...  A list of choices for user
PickFile()
{
  # Declare more meaningful names for args
  local result=""
  local returnname=$1
  local flag=$2
  local requireinput=$3
  local msg="$4"

  # Pick first existing file from choice list
  shift 4
  for file in $@
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
    echo -n "$msg"' ['"$result"']'
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
  eval "$returnname=$result"
  return 0
}

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

ShowLicense()
{
cat <<WELCOME_MSG
*********************************************************************
        Welcome to Google Sitemap Generator (Beta)!

For more information, please visit:
  http://code.google.com/p/googlesitemapgenerator/
*********************************************************************
WELCOME_MSG

ExecCmd 0 cat "$SRC_DIR/$WARN_FILE"
echo -n "Press ENTER to continue..."
read answer
echo ""

cat <<TOS_MSG
The product Terms of Service follows. You'll be asked to agree to the Terms of
Service at the end of many paragraphs. After installation is complete, you can
read the Terms of Service here:
TOS_MSG
echo "$DEST_DIR/$TOS_FILE"

echo -n "Press ENTER to continue..."
read answer

# Show ToS
ExecCmd 0 more -d "$SRC_DIR/$TOS_FILE"
echo ""
echo -n "Do you agree with the Terms of Service? [N/y]"
read answer
echo ""
if [ "x$answer" != "xy" -a "x$answer" != "xY" ]; then
  echo "The Google Sitemap Generator installation has been aborted."
  exit 1
fi

return 0
}

# Get options from command line.
CheckApache()
{
  if test "x$arg_apache_ctl" = "x"; then :; else
    apache_cmd="$arg_apache_ctl"
  fi
  if test "x$arg_apache_binary" = "x"; then :; else
    apache_cmd="$arg_apache_binary"
  fi

  if test "x$apache_cmd" = "x"; then
    cat <<CHECK_APACHE_MSG
This installation updates the Apache configuration file. To find that file,
the installer needs the location of the Apache binary (httpd) or control
script (apachectl). The binary or control script that you specify must
support the -V option.
CHECK_APACHE_MSG

    PickFile apache_cmd -f 1 \
    "What is the location of the Apache binary or control script?" \
    /usr/sbin/apache2 /usr/sbin/apache /usr/sbin/httpd \
    /usr/local/httpd/bin/httpd /usr/local/sbin/httpd
  fi

  while :
  do
    local valid_apache="true"
    SetApacheVar "APACHE_GROUP" "$arg_apache_group"
    SetApacheVar "APACHE_CONF" "$arg_apache_conf"
    SetApacheVar "APACHE_CTL" "$arg_apache_ctl"
    SetApacheVar "APACHE_BIN" "$arg_apache_binary"
    if AnalyzeApache "$apache_cmd"; then
      cat <<HERE_APACHE_MESSAGE
***************************************************************************
The following information about your Apache installation has been detected:
  * Apache version: $APACHE_VERSION
  * Apache architecture: $APACHE_ARCH bits
  * Apache root configuration file: $APACHE_CONF
  * Apache group: $APACHE_GROUP
***************************************************************************
HERE_APACHE_MESSAGE
      echo "Is all of this information correct? If you answer No, installation will"
      echo "terminate and you'll need to restart the installation, using the necessary"
      echo -n "command line options. [N/y]"
      read answer
      if [ "x$answer" != "xy" -a "x$answer" != "xY" ]; then
        valid_apache="false"
      fi
    else
      echo "$apache_cmd is not a supported Apache binary or control script."
      valid_apache="false"
    fi

    # Stop on valid apache.
    if [ "$valid_apache" = "true" ]; then
      break
    fi

    # Ask user to input a different location
    echo -n "Do you want to enter a different location for the Apache binary or control script? [Y/n]"
    read answer
    if [ "x$answer" = "xn" -o "x$answer" = "xN" ]; then
      echo "The Google Sitemap Generator installation has been aborted."
      exit 1
    else
      answer=""
      while [ -z "$answer" ]
      do
        echo -n "New location for the Apache binary or control script:"
        read answer
      done
      apache_cmd="$answer"
    fi
  done

  if SaveApacheInfo "$DEST_DIR/$APACHE_INFO"; then :; else
    echo "Failed to save apache.info"
    exit 1
  fi
}

GetInitialSetting()
{
  cat <<INITIAL_SETTING
Google Sitemap Generator will start creating Web Sitemap files as soon as it
starts up. Do you want Google Sitemap Generator to start submitting these
files automatically? There are three options:
1.  First installation. Start with automatic submission disabled.
2.  First installation. Start with automatic submission enabled.
3.  Reinstallation. Use the old automatic submission settings.

INITIAL_SETTING

  AUTO_SUBMISSION="1"
  while true; do
    echo -n "Specify your choice [$AUTO_SUBMISSION]:"
    read answer
    if test "x$answer" = "x"; then
      AUTO_SUBMISSION="1"
      break
    fi
    if test "x$answer" = "x1" ||
      test "x$answer" = "x2" ||
      test "x$answer" = "x3"; then
      AUTO_SUBMISSION=$answer;
      break
    fi
  done

  echo ""
  return 0
}

GetPreferences()
{
  # Get remote admin value.
  echo "You can now enable remote access for the Administration Console. \
Remote access lets you administer Google Sitemap Generator more easily \
but it is less secure than local access."

  echo -n "Do you want to enable remote access for Administration Console? [N/y]"
  read answer
  if [ "x$answer" != "xy" -a "x$answer" != "xY" ]; then
    REMOTE_ADMIN="false"
  else
    REMOTE_ADMIN="true"
    echo "Don't forget to configure your firewall after this installation is \
complete. You'll need allow remote access on the Google Sitemap Generator \
administration port."
  fi

  return 0
}

ChooseBinaryFiles()
{
# Prepare module file
local mod_name=mod_sitemap.$APACHE_VERSION.so
if test -f "$SRC_DIR/$mod_name"; then :; else
  echo "$SRC_DIR/$mod_name is missing."
  exit 1
fi

ExecCmd 0 rm -f "$SRC_DIR/mod_sitemap.so"
ExecCmd 0 cp "$SRC_DIR/$mod_name" "$SRC_DIR/mod_sitemap.so"

# Prepare daemon binary file.
local daemon_name=sitemap-daemon
if test -f "$SRC_DIR/$daemon_name"; then :; else
  echo "$SRC_DIR/$daemon_name is missing."
  exit 1
fi

# ExecCmd 0 rm -f "$SRC_DIR/sitemap-daemon"
# ExecCmd 0 cp "$SRC_DIR/$daemon_name" "$SRC_DIR/sitemap-daemon"

return 0;
}

# Install program files.
InstallProgramFiles()
{
umask 0

# make dir for the application to use.
ExecCmd 0 mkdir -p "$DEST_DIR/$BIN_DIR"
ExecCmd 0 mkdir -p "$DEST_DIR/$LIB_DIR"
ExecCmd 0 mkdir -p "$DEST_DIR/$RUN_DIR"
ExecCmd 0 mkdir -p "$DEST_DIR/$LOG_DIR"
ExecCmd 0 mkdir -p "$DEST_DIR/$ADM_DIR"
ExecCmd 0 mkdir -p "$DEST_DIR/$CONF_DIR"
ExecCmd 0 mkdir -p "$DEST_DIR/$REPO_DIR"
ExecCmd 0 mkdir -p "$DEST_DIR/$CGI_DIR"

# change permission flags for all the dirs.
ExecCmd 0 chmod 0755 "$DEST_DIR/$BIN_DIR"
ExecCmd 0 chmod 0755 "$DEST_DIR/$LIB_DIR"
ExecCmd 0 chmod 0755 "$DEST_DIR/$RUN_DIR"
ExecCmd 0 chmod 0755 "$DEST_DIR/$LOG_DIR"
ExecCmd 0 chmod 0755 "$DEST_DIR/$ADM_DIR"
ExecCmd 0 chmod 0755 "$DEST_DIR/$CONF_DIR"
ExecCmd 0 chmod 0755 "$DEST_DIR/$REPO_DIR"
ExecCmd 0 chmod 0755 "$DEST_DIR/$CGI_DIR"

# Save apache conf to a file for uninstall.
ExecCmd 0 echo $APACHE_CONF > $DEST_DIR/.apachebin
ExecCmd 0 chmod 644 "$DEST_DIR/.apachebin"

# Generate uninstall script
local temp_uninstall_sh=`mktemp /tmp/gsg_tmp.XXXXXX`
local escaped_bin=`echo $APACHE_BIN | sed 's/\//\\\\\//g'`
ExecCmd 0 sed "s/\"_APACHE_BIN_\"/$escaped_bin/" \
  $SRC_DIR/uninstall.sh > $temp_uninstall_sh

# Generate httpd.conf
local temp_httpd_conf=`mktemp /tmp/gsg_tmp.XXXXXX`
local escaped_gsg=`echo $DEST_DIR | sed 's/\//\\\\\//g'`
ExecCmd 0 sed "s/_GSG_ROOT_/$escaped_gsg/" \
  $SRC_DIR/httpd.conf > $temp_httpd_conf

# Copy files
ExecCmd 0 mv -f $temp_uninstall_sh "$DEST_DIR/$UNINSTALL_SCRIPT"
ExecCmd 0 cp -f "$SRC_DIR/$DAEMON_BIN" "$DEST_DIR/$BIN_DIR"
ExecCmd 0 cp -f "$SRC_DIR/$ADMIN_BIN" "$DEST_DIR/$CGI_DIR/$CGI_BIN"
ExecCmd 0 cp -rf "$SRC_DIR/$ADM_DIR/*" "$DEST_DIR/$ADM_DIR"
ExecCmd 0 cp -f "$SRC_DIR/$APACHE_SCRIPT" "$DEST_DIR/$BIN_DIR"
ExecCmd 0 cp -f "$SRC_DIR/$TOS_FILE" "$DEST_DIR"
ExecCmd 0 cp -f "$temp_httpd_conf" "$DEST_DIR/$CONF_DIR/$HTTPD_CONF"

# Remove temp files.
ExecCmd 1 rm -f $temp_httpd_conf
ExecCmd 1 rm -f $temp_uninstall_sh

# Copy module file according to whether SELinux is enabled.
if which selinuxenabled 1>/dev/null 2>/dev/null; then
  local context="system_u:object_r:httpd_modules_t"
  echo "SELinux is enabled."
  echo "Change security context of $MODULE_FILE to $context"
  ExecCmd 0 install -Z $context "$SRC_DIR/$MODULE_LIB" "$DEST_DIR/$LIB_DIR"
else
  ExecCmd 0 cp -f "$SRC_DIR/$MODULE_LIB" "$DEST_DIR/$LIB_DIR"
fi

# Change directory permissions.
ExecCmd 0 find "$DEST_DIR/$ADM_DIR" -type d -exec chmod 755 {} \;
ExecCmd 0 find "$DEST_DIR/$ADM_DIR" -type f -exec chmod 644 {} \;

# Change file permissions
ExecCmd 0 chmod 0750 "$DEST_DIR/$UNINSTALL_SCRIPT"
ExecCmd 0 chmod 0750 "$DEST_DIR/$BIN_DIR/$APACHE_SCRIPT"
ExecCmd 0 chmod 0750 "$DEST_DIR/$BIN_DIR/$DAEMON_BIN"
ExecCmd 0 chmod 0755 "$DEST_DIR/$LIB_DIR/$MODULE_LIB"
ExecCmd 0 chmod 0755 "$DEST_DIR/$CGI_DIR/$CGI_BIN"
ExecCmd 0 chmod 0644 "$DEST_DIR/$CONF_DIR/$HTTPD_CONF"
ExecCmd 0 chmod 0644 "$DEST_DIR/$TOS_FILE"

# Make a link to daemon binary.
ExecCmd 0 ln -fs "$DEST_DIR/$BIN_DIR/$DAEMON_BIN" "$DAEMON_LINK"

# Save home configuration.
ExecCmd 0 mkdir -p "$GLOBAL_CONF_DIR"
ExecCmd 0 echo "$DEST_DIR" > "$GLOBAL_CONF_DIR/$GLOBAL_CONF_HOME"
ExecCmd 0 chmod 644 "$GLOBAL_CONF_DIR/$GLOBAL_CONF_HOME"


echo "Program files successfully copied."
return 0;
}

# update site settings
UpdateSiteSettings()
{
  update_setting_flags=""
  if test $AUTO_SUBMISSION = "1"; then
    update_setting_flags="overwrite=true auto_submission=false"
  elif test $AUTO_SUBMISSION = "2"; then
    update_setting_flags="overwrite=true auto_submission=true"
  elif test $AUTO_SUBMISSION = "3"; then
    update_setting_flags="overwrite=false"
  fi

  if "$DEST_DIR/$BIN_DIR/$DAEMON_BIN" update_setting \
  $update_setting_flags \
  "apache_conf=$APACHE_CONF" "apache_group=$APACHE_GROUP" \
  1>/dev/null; then :; else
    echo "Failed to update site settings."
    exit 1
  fi

  echo "Google Sitemap Generator settings successfully updated."
  return 0
}

# Install init script to auto start sitemapservice automatically
InstallInitScript_Linux()
{
  if test -d "/etc/init.d"; then :; else
    echo "WARNING: No such directory: (/etc/init.d)"
    echo "WARNING: Init script will not be installed."
    return 0
  fi

  local service_file="/etc/init.d/$PKG_NAME"

  # check whether a same name autostart script exist
  if [ -f "$service_file" ]; then
    echo "WARNING: There is already an init script named as [$PKG_NAME]"
    echo -n "WARNING: Do you want to proceed any way? [N/y]"
    read answer
    if [ "$answer" = N -o "$answer" = n -o -z "$answer" ]; then
      return 1
    fi
  fi

  # retrieve current run level
  # RUNLEVEL=$(runlevel | awk '{print $2}')
  # echo "[Warn] Init service will be installed at level $RUNLEVEL"

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
    echo "WARNING: System startup scripts link dir can't be found."
    echo "WARNING: Auto-start scripts are not installed."
    return 0
  fi

  ExecCmd 1 ln -fs $service_file $rc_parent/rc0.d/K10$PKG_NAME
  ExecCmd 1 ln -fs $service_file $rc_parent/rc1.d/K10$PKG_NAME
  ExecCmd 1 ln -fs $service_file $rc_parent/rc2.d/K10$PKG_NAME
  ExecCmd 1 ln -fs $service_file $rc_parent/rc3.d/K10$PKG_NAME
  ExecCmd 1 ln -fs $service_file $rc_parent/rc4.d/K10$PKG_NAME
  ExecCmd 1 ln -fs $service_file $rc_parent/rc5.d/K10$PKG_NAME
  ExecCmd 1 ln -fs $service_file $rc_parent/rc6.d/K10$PKG_NAME

  ExecCmd 1 ln -fs $service_file $rc_parent/rc2.d/S90$PKG_NAME
  ExecCmd 1 ln -fs $service_file $rc_parent/rc3.d/S90$PKG_NAME
  ExecCmd 1 ln -fs $service_file $rc_parent/rc4.d/S90$PKG_NAME
  ExecCmd 1 ln -fs $service_file $rc_parent/rc5.d/S90$PKG_NAME

  return 0
}

InstallInitScript_FreeBSD()
{
  local service_file="/etc/rc.local"
  if echo "/usr/sbin/google-sitemap-generator-ctl -start" >> $service_file; \
  then :; else
    echo "WARINING: Failed to install the service to /etc/rc.local"
  fi

  return 0
}

InstallInitScript()
{
  local os_name=`uname`
  if [ "x$os_name" = "xLinux" ]; then
    if InstallInitScript_Linux; then :; else
      return 1
    fi
  elif [ "x$os_name" = "xFreeBSD" ]; then
    if InstallInitScript_FreeBSD; then :; else
      return 1
    fi
  else
    echo "$os_name is unsupported."
    return 0
  fi

  echo "Google Sitemap Generator init scripts successfully installed."
  return 0
}

# install sitemap apache module
ModifyApacheConf()
{
  # update the configuration file
  local temp_conf=`mktemp /tmp/gsg_tmp.XXXXXX`
  ExecCmd 0 cp -f "$APACHE_CONF" "$GLOBAL_CONF_DIR/httpd.install.conf"
  ExecCmd 0 cp -f "$APACHE_CONF" "$temp_conf"
  ExecCmd 0 echo "Include $DEST_DIR/$CONF_DIR/$HTTPD_CONF" >> $temp_conf
  ExecCmd 0 mv -f $temp_conf "$APACHE_CONF"

  echo "Apache configuration successfully updated."
  echo "Old configuration is saved at $GLOBAL_CONF_DIR/httpd.install.conf"
  echo ""
  return 0
}

SetAdminPassword()
{
  echo "Ready to set the password for the administration console."
  if "$DEST_DIR/$BIN_DIR/$DAEMON_BIN" reset_password; then :; else
    echo "Failed to set admin password."
    exit 1
  fi
  echo ""
  return 0
}

PostInstall()
{
  # Start service.
  if "$DAEMON_LINK" service start 1>/dev/null; then
    echo "Google Sitemap Generator daemon successfully started."
  else
    echo "Google Sitemap Generator daemon can't be started."
    exit 1
  fi

  # Restart apache.
  echo "To start the Google Sitemap Generator module in Apache, \
you must restart Apache."
  if [ "x$APACHE_CTL" = "x" ]; then :; else

    echo -n "Do you want to restart Apache now? [N/y]"
    read answer
    echo ""
    if [ "x$answer" = "xy" -o "x$answer" = "xY" ]; then
      if RestartApache; then
        echo "Google Sitemap Generator (Beta) was successfully installed."
        echo "Apache was successfully restarted."
        echo "You can now go to http://<this-server-address>:8181/ to start configuration."
        return 0
      else
        echo "Failed to restart Apache. You need restart it manually."
      fi
    fi
  fi

  cat <<HERE_COMPLETE_MSG
After you restart Apache, you can go to http://<this-server-address>:8181/ to
configure the application.
Google Sitemap Generator (Beta) was successfully installed.
HERE_COMPLETE_MSG
  return 0
}

Help()
{
  cat <<HERE_HELP
Usage: install.sh [-h] [-d directory] [-a file]
                  [-c file] [-g group] [-t file]
Options:
  -h             : list available command line options (this page)
  -d directory   : specifiy an alternative installation target directory
  -a file        : specify the Apache binary file path
  -c file        : specify the Apache root configuration file
  -g group       : specify the Apache running Group name
  -t file        : specify the Apache control script
HERE_HELP
}

# ---------------------------------------------------------
# ensure user is root
if [ `id -u` != "0" ]; then
  echo "Permission denied. Installation script must be run with superuser privileges."
  exit 1
fi

# Parse args
if getopt ha:d:g:c:t: $@ 1>/dev/null 2>/dev/null ; then
  args=`getopt ha:d:g:c:t: $@`
else
  echo "Use $0 -h to get more information"
  exit 1
fi

set -- $args
while true; do
  case "$1" in
    -h)     Help; exit 0 ;;
    -d)     PREFIX="$2"; DEST_DIR="$PREFIX/$PKG_NAME" shift 2 ;;
    -a)     arg_apache_binary="$2"; shift 2 ;;
    -g)     arg_apache_group="$2"; shift 2 ;;
    -c)     arg_apache_conf="$2"; shift 2;;
    -t)     arg_apache_ctl="$2"; shift 2;;
    --)     shift; break ;;
    *)      echo "Invalid option $1, use $0 -h to get mroe information"
            exit 1;;
  esac
done

GetSrcDir

# import apache contorl script
if test -f "$SRC_DIR/apache.sh" ; then :; else
  echo "($SRC_DIR/apache.sh) can't be found."
  exit 1
fi
. "$SRC_DIR/apache.sh"

CheckOldInstallation

ShowLicense

InitDestDir

CheckApache

# GetPreferences
GetInitialSetting

ChooseBinaryFiles

InstallProgramFiles

UpdateSiteSettings

InstallInitScript

ModifyApacheConf

SetAdminPassword

PostInstall

exit 0
