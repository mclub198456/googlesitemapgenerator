#!/bin/sh
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

APACHE_BIN=""
APACHE_CONF=""
APACHE_VERSION=""
APACHE_PIDFILE=""
APACHE_HTTPD_ROOT=""
APACHE_SERVER_ROOT=""
APACHE_ARCH=32
CHECK_FLAG=1


APACHE_PID=0
APACHE_RUNNING=0

AnalyzeApache()
{
  APACHE_BIN="$1"

  if test "x$APACHE_BIN" = "x"; then
    echo "Apache bin shouldn't be empty"
    return 1
  fi

  if test -x "$APACHE_BIN"; then :; else
    echo "[$APACHE_BIN] should be an executable file."
    return 2
  fi

  if "$APACHE_BIN" -V 1>/dev/null 2>/dev/null; then :; else
    echo "[$APACHE_BIN] should support -V option."
    return 3
  fi

  APACHE_VERSION=`"$APACHE_BIN" -V | sed 's/[://]/ /g' \
    | awk '/version/ { print $4 }' | awk -F. '{ printf("%s.%s", $1, $2) }'`

  if test "x$APACHE_VERSION" != "x1.3" && \
    test "x$APACHE_VERSION" != "x2.0" && \
    test "x$APACHE_VERSION" != "x2.2"; then
    echo "[$APACHE_BIN] is not a supported apache binary."
    return 4
  fi

  if test "x$APACHE_VERSION" = "x1.3" ; then
    EAPI=`"$APACHE_BIN" -V | awk '/EAPI/ { print $2 }'`
    if test "x$EAPI" = "xEAPI" ; then
      APACHE_VERSION="1.3.e"
    fi
  fi

  # Retrieve -D HTTPD_ROOT=".."
  APACHE_HTTPD_ROOT=`"$APACHE_BIN" -V | sed 's/=/ /g' \
    | awk '/HTTPD_ROOT/ { print $3 }' | sed 's/"//g'`

  # Retrieve -D SERVER_CONFIG_FILE=".."
  APACHE_CONF=`"$APACHE_BIN" -V | sed 's/=/ /g' \
    | awk '/SERVER_CONFIG_FILE/ { print $3 }' | sed 's/"//g'`
  if test "x$APACHE_CONF" = "x"; then
    echo "[$APACHE_BIN -V] doens't contain SERVER_CONFIG_FILE value."
    return 5
  fi

  # Make APACHE_CONF as an absolute path.
  local temp_str=`echo "$APACHE_CONF" | grep '^/'`
  if test "x$temp_str" = "x" ; then
    APACHE_CONF="$APACHE_HTTPD_ROOT/$APACHE_CONF"
  fi

  if cat "$APACHE_CONF" 1>/dev/null 2>/dev/null; then :; else
    echo "[$APACHE_CONF] is not a readable apache configuration file."
    return 6
  fi

  # Determine ServerRoot directive.
  temp_str=`cat "$APACHE_CONF" | grep '^[[:blank:]]*ServerRoot'`
  if test "x$temp_str" != "x" ; then
    APACHE_SERVER_ROOT=`echo "$temp_str" | \
    grep '^[[:blank:]]*ServerRoot[[:blank:]]*' | awk -F'"' ' { print $2 } '`
    if test "x$APACHE_SERVER_ROOT" = "x" ; then
      APACHE_SERVER_ROOT=`echo "$temp_str" | awk ' { print $2 } '`
    fi
  fi
  if test "x$APACHE_SERVER_ROOT" = "x" ; then
    APACHE_SERVER_ROOT="$APACHE_HTTPD_ROOT"
  fi
  temp_str=`echo "$APACHE_SERVER_ROOT" | grep '^/'`
  if test "x$temp_str" = "x" ; then
    echo "[$APACHE_SERVER_ROOT] is not an absolute path"
    return 7
  fi

  # Determine PidFile directive.
  temp_str=`cat "$APACHE_CONF" | grep '^[[:blank:]]*PidFile'`
  if test "x$temp_str" != "x" ; then
    APACHE_PIDFILE=`echo "$temp_str" | \
    grep '^[[:blank:]]*PidFile[[:blank:]]*' | awk -F'"' ' { print $2 } '`
    if test "x$APACHE_PIDFILE" = "x" ; then
      APACHE_PIDFILE=`echo "$temp_str" | awk ' { print $2 } '`
    fi
  fi
  if test "x$APACHE_PIDFILE" = "x" ; then
    APACHE_PIDFILE=`"$APACHE_BIN" -V | sed 's/=/ /g' \
      | awk '/DEFAULT_PIDLOG/ { print $3 }' | sed 's/"//g'`
  fi

  # Check and normalize PidFile directive.
  if test "x$APACHE_PIDFILE" = "x" ; then
    echo "Can't determine PidFile directive for apache."
    return 8
  fi
  temp_str=`echo "$APACHE_PIDFILE" | grep '^/'`
  if test "x$temp_str" = "x" ; then
    APACHE_PIDFILE="$APACHE_SERVER_ROOT/$APACHE_PIDFILE"
  fi

  APACHE_ARCH=32
  if "$APACHE_BIN" -V | grep Architecture | grep 64 1>/dev/null 2>/dev/null; then
    line=`"$APACHE_BIN" -V | grep Architecture | grep 64`
    if test "x$line" != "x"; then
      APACHE_ARCH=64
    fi
  fi

  CHECK_FLAG=0
  return 0
}

ApacheStatus()
{
  if test $CHECK_FLAG -eq 1; then
    echo "Please run AnalyzeApache first"
    return 1
  fi

  if test -f "$APACHE_PIDFILE"; then
    APACHE_PID=`cat "$APACHE_PIDFILE"`
    if test "x$APACHE_PID" != "x" && kill -0 $APACHE_PID 2>/dev/null ; then
      APACHE_RUNNING=1
    else
      APACHE_RUNNING=0
    fi
  else
    APACHE_RUNNING=0
  fi

  return 0
}

StartApache()
{
  if ApacheStatus ; then :; else
    return 1
  fi

  if test "x$APACHE_VERSION" = "x1.3" ||
    test "x$APACHE_VERSION" = "x1.3.e" ; then
    if test $APACHE_RUNNING -eq 1; then
      echo "Apache already running."
      return 0;
    fi
    if "$APACHE_BIN" ; then
      echo "Apache started."
      return 0;
    else
      echo "Apache could not be started."
      return 2;
    fi
  else
    "$APACHE_BIN" -k start
    return $?
  fi
}

StopApache()
{
  if ApacheStatus ; then :; else
    return 1
  fi

  if test "x$APACHE_VERSION" = "x1.3" ||
    test "x$APACHE_VERSION" = "x1.3.e" ;  then
    if test $APACHE_RUNNING -eq 0; then
      echo "Apache is not running."
      return 0;
    fi
    if kill $APACHE_PID ; then
      echo "Apache is stopped."
      return 0;
    else
      echo "Apache could not be stopped."
      return 2;
    fi
  else
    "$APACHE_BIN" -k stop
    return $?
  fi
}

RestartApache()
{
  if ApacheStatus ; then :; else
    return 1
  fi

  if test "x$APACHE_VERSION" = "x1.3" ||
    test "x$APACHE_VERSION" = "x1.3.e" ; then
    if [ $APACHE_RUNNING -eq 0 ]; then
      if $APACHE_BIN ; then
        echo "Apache was not running, and is started"
        return 0
      else
        echo "Apache couldn't be started."
        return 2
      fi
    else
      if kill -USR1 $APACHE_PID ; then
        echo "Apache is restarted successfully."
        return 0
      else
        echo "Failed to restart Apache."
        return 3
      fi
    fi
  else
    if "$APACHE_BIN" -k graceful; then
      echo "Apache is restarted successfully."
      return 0
    else
      echo "Failed to restart Apache."
      return 2
    fi
    return $?
  fi
}

# AnalyzeApache "/usr/sbin/httpd"
# echo "HTTPD_ROOT: $APACHE_HTTPD_ROOT"
# echo "ServerRoot: $APACHE_SERVER_ROOT"
# echo "PidFile: $APACHE_PIDFILE"
# echo "ConfigFile: $APACHE_CONF"
# echo "Apache Arch: $APACHE_ARCH"
