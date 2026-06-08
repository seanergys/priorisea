// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <cxxabi.h>
#include <memory>
#include <string>

////

template <typename T> std::string ps_type_name() {
  int status = 0;
  std::unique_ptr<char, void (*)(void *)> res{
      abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status),
      std::free};
  return (status == 0) ? res.get() : typeid(T).name();
}

template <typename T> std::string ps_type_name(const T &value) {
  int status = 0;
  std::unique_ptr<char, void (*)(void *)> res{
      abi::__cxa_demangle(typeid(value).name(), nullptr, nullptr, &status),
      std::free};
  return (status == 0) ? res.get() : typeid(T).name();
}

////

bool parse_int(const std::string &s, int &out);

bool parse_double(const std::string &s, double &out);
