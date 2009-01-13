// Copyright 2009 Google Inc.
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


#include "sitemapservice/passwordmanager.h"

#if defined(WIN32)
#include <conio.h>

#elif defined(__linux__) || defined(__unix__)
#include <termios.h>
#include <stdio.h>

#endif

#include "common/logger.h"
#include "common/port.h"
#include "common/util.h"
#include "common/sitesettings.h"

bool PasswordManager::ChangePassword() {
  if (!Login()) {
    return false;
  }
  return CreatePassword();
}

bool PasswordManager::Login() {
  // Get password from command line.
  std::string passwd;
  printf("Enter current password:");
  if (!GetInput(&passwd)) {
    return false;
  }
  printf("\n");

  LoginResult result = Login(passwd);
  if (result == LOGIN_FAIL) {
    fprintf(stderr, "Internal application error.");
    return false;
  } else if (result == LOGIN_ERROR) {
    fprintf(stderr, "Invalid password.");
    return false;
  }
  return true;
}

PasswordManager::LoginResult PasswordManager::Login(const std::string& password) {
  SiteSettings settings;
  if (!SettingManager::default_instance()->LoadApplicationSetting(&settings)) {
    Logger::Log(EVENT_ERROR, "Failed to retrieve password.\n");
    return LOGIN_ERROR;
  }

  if (settings.admin_password().length() == 0) {
    Logger::Log(EVENT_ERROR, "Password has not been set.");
    return LOGIN_ERROR;
  }

  // Append salt to user input, and hash it.
  std::string passwd(password);
  passwd.append(settings.password_salt());
  std::string encrypted;
  if (!Util::MD5Encrypt(passwd.c_str(), &encrypted)) {
    Logger::Log(EVENT_ERROR, "Failed to encrypt your password.\n");
    return LOGIN_ERROR;
  }

  if (encrypted != settings.admin_password()) {
    Logger::Log(EVENT_ERROR, "Invalid password.\n");
    return LOGIN_FAIL;
  }

  return LOGIN_OK;
}

bool PasswordManager::CreatePassword() {
  // Get raw password from user input.
  std::string passwd;
  if (!GetNewPassword(&passwd)) {
    return false;
  }

  if (!ChangePassword(passwd)) {
    fprintf(stderr, "Failed to save new password.");
    return false;
  }

  return true;
}

bool PasswordManager::ChangePassword(const std::string &password) {
  // Make a writable copy.
  std::string passwd = password;

  // Get salt for password.
  char salt_buf[32];
  int salt_size = 32;
  if (!Util::GenerateRandom(salt_buf, salt_size)) {
    Logger::Log(EVENT_CRITICAL, "Simple salt will be used.");
    
    itoa(static_cast<int>(time(NULL)), salt_buf);
    salt_size = static_cast<int>(strlen(salt_buf));
  }

  std::string salt;
  if (!Util::MD5Encrypt(salt_buf, salt_size, &salt)) {
    Logger::Log(EVENT_ERROR, "Failed to hash salt bytes.");
    return false;
  }

  // Append salt to raw password, and encrypt it.
  passwd.append(salt);
  std::string encrypted;
  if (!Util::MD5Encrypt(passwd.c_str(), &encrypted)) {
    Logger::Log(EVENT_ERROR, "Failed to encrypt new password.\n");
    return false;
  }

  if (!SettingManager::default_instance()->SetPassword(
                       encrypted.c_str(), salt.c_str())) {
    Logger::Log(EVENT_ERROR, "Failed to save new password.\n");
    return false;
  }

  return true;
}

bool PasswordManager::GetNewPassword(std::string* passwd) {
  while (true) {
    std::string new_pwd, repeat_pwd;

    printf("Password (5 or more characters):");
    if (!GetInput(&new_pwd)) {
      return false;
    }
    printf("\n");

    printf("Confirm password:");
    if (!GetInput(&repeat_pwd)) {
      return false;
    }
    printf("\n");

    if (new_pwd != repeat_pwd) {
      fprintf(stderr, "The passwords you entered do not match. Please try again.\n");
    } else if (new_pwd.length() < 5) {
      fprintf(stderr, "Minimum length of password is 5.\n");
    } else {
      passwd->assign(new_pwd);
      return true;
    }
  }

}


#if defined(WIN32)

bool PasswordManager::GetInput(std::string* input) {
  input->clear();
  while (true) {
    int c = _getch();
    if (c == '\n' || c == '\r') {
      return true;
    }
    _putch('*');
    input->push_back(static_cast<char>(c));
  }
  return true;
}

#elif defined(__linux__) || defined(__unix__)

bool PasswordManager::GetInput(std::string* input) {
  struct termios old_ios, new_ios;

  // Turn off echoing.
  if (tcgetattr(fileno(stdin), &old_ios) != 0) {
    return false;
  }
  new_ios = old_ios;
  new_ios.c_lflag &= ~ECHO;

  if (tcsetattr(fileno(stdin), TCSAFLUSH, &new_ios) != 0) {
    return false;
  }

  // Get password.
  input->clear();
  while (true) {
    int c = getchar();
    if (c != '\n' && c != EOF) {
      input->push_back(static_cast<char>(c));
    } else {
      break;
    }
  }

  // Restore echoing.
  tcsetattr(fileno(stdin), TCSAFLUSH, &old_ios);
  return true;
}

#endif
