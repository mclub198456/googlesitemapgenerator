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

# sitemapservice  This init.d script is used to start sitemapservice
#                 It basically just calls /usr/sbin/sitemapservice

SITEMAPSERVICE="/usr/sbin/google-sitemap-generator-ctl"
DATA_DIR=/var/spool/google-sitemap-generator

# check wether the service exist
if [ -x "$SITEMAPSERVICE" ]; then
  HAVE_SITEMAPSERVICE=1
else
  echo "Sitemapservicectl can't be found."
  exit 0
fi

# return value
RETVAL=0

case $1 in
  start)  # init S param
    [ ! -d $DATA_DIR ] && mkdir $DATA_DIR
    echo "Starting sitemap service..."
    $SITEMAPSERVICE -start
    RETVAL=$?
  ;;
  stop)  # init K param
    echo "Stopping sitemap service..."
    $SITEMAPSERVICE -stop
    RETVAL=$?
    rm -f "/var/lock/google-sitemap-generator/*.lck"
  ;;
esac

exit $RETVAL
