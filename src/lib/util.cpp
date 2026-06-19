// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "util.hpp"
#include <cmath>
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
