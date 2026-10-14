// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/lib/util.hpp"
#include <cmath>
#include <pwd.h>
#include <stdexcept>

bool parse_int(const std::string &s, int &out) {
  try {
    size_t pos = 0;
    int value = std::stoi(s, &pos);
    if (pos != s.size()) {
      return false;
    }
    out = value;
    return true;
  } catch (const std::invalid_argument &) {
    return false;
  } catch (const std::out_of_range &) {
    return false;
  }
}

bool parse_uid(const std::string &s, uid_t &out) {
  char *end = nullptr;
  errno = 0;
  unsigned long value = std::strtoul(s.c_str(), &end, 10);
  if (errno != 0 || *end != '\0') {
    return false;
  }
  if (value > std::numeric_limits<uid_t>::max()) {
    return false;
  }
  out = static_cast<uid_t>(value);
  return true;
}

bool parse_double(const std::string &s, double &out) {
  try {
    size_t pos = 0;
    double value = std::stod(s, &pos);
    if (pos != s.size()) {
      return false;
    }
    if (!std::isfinite(value)) {
      return false;
    }
    out = value;
    return true;
  } catch (const std::invalid_argument &) {
    return false;
  } catch (const std::out_of_range &) {
    return false;
  }
}

bool parse_uint64(const std::string &s, uint64_t &out) {
  try {
    size_t pos = 0;
    unsigned long long value = std::stoull(s, &pos);
    if (pos != s.size()) {
      return false;
    }
    out = static_cast<uint64_t>(value);
    return true;
  } catch (const std::invalid_argument &) {
    return false;
  } catch (const std::out_of_range &) {
    return false;
  }
}

bool uid_from_username(const std::string &name, uid_t &out) {
  struct passwd *pw = getpwnam(name.c_str());
  if (!pw) {
    return false;
  }
  out = pw->pw_uid;
  return true;
}

bool username_from_uid(uid_t uid, std::string &out) {
  struct passwd *pw = getpwuid(uid);
  if (!pw) {
    return false;
  }
  out = pw->pw_name;
  return true;
}
