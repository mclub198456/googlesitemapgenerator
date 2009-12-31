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

APACHE_BIN=""
APACHE_CONF=""
APACHE_VERSION=""
APACHE_HTTPD_ROOT=""
APACHE_SERVER_ROOT=""
APACHE_ARCH=32
APACHE_GROUP=""
APACHE_CTL=""
CHECK_FLAG=1

# Param1: file to load config
# Param2: directive name
# Param3: name of return value
ParseDirective()
{
  local file="$1"
  local name="$2"
  local result=""

  pd_temp_str=`cat "$file" | grep "^[[:blank:]]*$name"`
  if test "x$pd_temp_str" != "x" ; then
    result=`echo "$pd_temp_str" | \
    grep "^[[:blank:]]*$name[[:blank:]]*" | awk -F'"' ' { print $2 } '`
    if test "x$result" = "x" ; then
      result=`echo "$pd_temp_str" | awk ' { print $2 } '`
    fi
    result=`echo $result | tr -d '\r'`
  fi

  eval "$3=\"$result\""
  return 0
}

# Param1: var name
# Param2: var value
SetApacheVar()
{
  eval "$1=\"$2\""
  return 0
}

AnalyzeApache()
{
  APACHE_CMD="$1"

  if test "x$APACHE_CMD" = "x"; then
    echo "Apache binary or control script shouldn't be empty."
    return 1
  fi

  if test -x "$APACHE_CMD"; then :; else
    echo "[$APACHE_CMD] should be an executable file."
    return 2
  fi

  if "$APACHE_CMD" -V 1>/dev/null 2>/dev/null; then :; else
    echo "[$APACHE_CMD] should support -V option."
    return 3
  fi

  APACHE_VERSION=`"$APACHE_CMD" -V | sed 's/[://]/ /g' \
    | awk '/version/ { print $4 }' | awk -F. '{ printf("%s.%s", $1, $2) }'`

  if test "x$APACHE_VERSION" != "x1.3" && \
    test "x$APACHE_VERSION" != "x2.0" && \
    test "x$APACHE_VERSION" != "x2.2"; then
    echo "[$APACHE_CMD] doesn't point to a supported Apache instance."
    return 4
  fi

  if test "x$APACHE_VERSION" = "x1.3" ; then
    EAPI=`"$APACHE_CMD" -V | awk '/EAPI/ { print $2 }'`
    if test "x$EAPI" = "xEAPI" ; then
      APACHE_VERSION="1.3.e"
    fi
  fi

  # Retrieve -D HTTPD_ROOT=".."
  APACHE_HTTPD_ROOT=`"$APACHE_CMD" -V | sed 's/=/ /g' \
    | awk '/HTTPD_ROOT/ { print $3 }' | sed 's/"//g'`

  # Get value for APACHE_CONF
  if test "x$APACHE_CONF" = "x"; then
    # Retrieve -D SERVER_CONFIG_FILE=".."
    APACHE_CONF=`"$APACHE_CMD" -V | sed 's/=/ /g' \
      | awk '/SERVER_CONFIG_FILE/ { print $3 }' | sed 's/"//g'`
    if test "x$APACHE_CONF" = "x"; then
      echo "[$APACHE_CMD -V] doens't contain SERVER_CONFIG_FILE value."
      return 5
    fi

    # Make APACHE_CONF as an absolute path.
    local temp_str=`echo "$APACHE_CONF" | grep '^/'`
    if test "x$temp_str" = "x" ; then
      APACHE_CONF="$APACHE_HTTPD_ROOT/$APACHE_CONF"
    fi
  fi

  if cat "$APACHE_CONF" 1>/dev/null 2>/dev/null; then :; else
    echo "[$APACHE_CONF] is not a readable apache configuration file."
    return 6
  fi

  # Determine ServerRoot directive.
  ParseDirective "$APACHE_CONF" "ServerRoot" "APACHE_SERVER_ROOT"
  if test "x$APACHE_SERVER_ROOT" = "x" ; then
    APACHE_SERVER_ROOT="$APACHE_HTTPD_ROOT"
  fi
  temp_str=`echo "$APACHE_SERVER_ROOT" | grep '^/'`
  if test "x$temp_str" = "x" ; then
    echo "ServerRoot [$APACHE_SERVER_ROOT] is not an absolute path."
    return 7
  fi

  # Get value for APACHE_GROUP
  if test "x$APACHE_GROUP" = "x"; then
    ParseDirective "$APACHE_CONF" "Group" "APACHE_GROUP"
    if test "x$APACHE_GROUP" = "x" ; then
      echo "Can't determine Group directive for Apache."
      return 8
    fi
  fi

  # Get value for APACHE_ARCH
  APACHE_ARCH=32
  if "$APACHE_CMD" -V | grep Architecture | grep 64 1>/dev/null 2>/dev/null; then
    line=`"$APACHE_CMD" -V | grep Architecture | grep 64`
    if test "x$line" != "x"; then
      APACHE_ARCH=64
    fi
  fi

  CHECK_FLAG=0
  return 0
}

RestartApache()
{
  if test "x$APACHE_CTL" = "x" ; then
    echo "Unable to find Apache control script."
    return 2
  fi

  if test "x$APACHE_VERSION" = "x1.3" ||
    test "x$APACHE_VERSION" = "x.1.3.e" ; then
    if "$APACHE_CTL" graceful; then
      return 0
    else
      return 1
    fi
  else
    if "$APACHE_CTL" -k graceful; then
      return 0
    else
      return 1
    fi
  fi
}

SaveApacheInfo()
{
  local info_file="$1"
  echo "APACHE_CONF $APACHE_CONF" > $info_file
  echo "APACHE_VERSION $APACHE_VERSION" >> $info_file
  echo "APACHE_ARCH $APACHE_ARCH" >> $info_file
  echo "APACHE_BIN $APACHE_BIN" >> $info_file
  echo "APACHE_GROUP $APACHE_GROUP" >> $info_file
  echo "APACHE_CTL $APACHE_CTL" >> $info_file

  return 0
}

LoadApacheInfo()
{
  local info_file="$1"
  ParseDirective "$info_file" "APACHE_CTL" "APACHE_CTL"
  ParseDirective "$info_file" "APACHE_CONF" "APACHE_CONF"
  ParseDirective "$info_file" "APACHE_VERSION" "APACHE_VERSION"
  ParseDirective "$info_file" "APACHE_ARCH" "APACHE_ARCH"
  ParseDirective "$info_file" "APACHE_BIN" "APACHE_BIN"
  ParseDirective "$info_file" "APACHE_GROUP" "APACHE_GROUP"

  CHECK_FLAG=0
  return 0
}


PrintApacheInfo()
{
  echo "HTTPD_ROOT: $APACHE_HTTPD_ROOT"
  echo "ServerRoot: $APACHE_SERVER_ROOT"
  echo "ConfigFile: $APACHE_CONF"
  echo "Apache Arch: $APACHE_ARCH"
  echo "Apache Version: $APACHE_VERSION"
  echo "Apache Bin: $APACHE_BIN"
  echo "Apache Group: $APACHE_GROUP"
}

