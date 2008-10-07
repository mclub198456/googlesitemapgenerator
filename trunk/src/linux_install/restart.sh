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


APACHE="_APACHE_BIN_"

# restart service
/usr/sbin/google-sitemap-generator-ctl -restart $@ || exit $?

# Check whether the apache server should be started.
if [ "x$1" != "xall" ]; then
  exit 0
fi

# import apache control script.
MYDIR=`dirname "$0"`
if test -f "$MYDIR/apache.sh" ; then :; else
  echo "($MYDIR/apache.sh) can't be found."
  exit 101
fi
. "$MYDIR/apache.sh"

# Restart apache server.
if AnalyzeApache "$APACHE"; then :; else
  echo "($APACHE) is an invalid apache binary."
  exit 102
fi

if RestartApache; then
  echo "Apache is restarted successfully."
else
  echo "Failed to restart apache."
  exit 103
fi

exit 0
