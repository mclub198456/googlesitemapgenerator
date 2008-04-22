@echo off

REM Copyright 2008 Google Inc.
REM
REM Licensed under the Apache License, Version 2.0 (the "License");
REM you may not use this file except in compliance with the License.
REM You may obtain a copy of the License at
REM
REM      http://www.apache.org/licenses/LICENSE-2.0
REM
REM Unless required by applicable law or agreed to in writing, software
REM distributed under the License is distributed on an "AS IS" BASIS,
REM WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
REM See the License for the specific language governing permissions and
REM limitations under the License.
REM
REM Batch file to restart GoogleSitemapService and w3svc
REM


net stop GoogleSitemapGenerator
IF %ERRORLEVEL% NEQ 0 EXIT /B 1

IF "%1" EQU "all" (
  net stop w3svc
  IF %ERRORLEVEL% NEQ 0 EXIT /B 3

  net start w3svc
  IF %ERRORLEVEL% NEQ 0 EXIT /B 4
)

net start GoogleSitemapGenerator
IF %ERRORLEVEL% NEQ 0 EXIT /B 2

EXIT /B 0

