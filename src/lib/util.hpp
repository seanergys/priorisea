// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <cxxabi.h>
#include <memory>
#include <string>
#include <sys/types.h>

/**
 * Extract the type name of a variable or object.
 *
 * @param value The variable or object to extract the type name from.
 * @return The type name as a string.
 */
template <typename T> std::string ps_type_name(const T &value) {
  int status = 0;
  std::unique_ptr<char, void (*)(void *)> res{
      abi::__cxa_demangle(typeid(value).name(), nullptr, nullptr, &status),
      std::free};
  return (status == 0) ? res.get() : typeid(T).name();
}

/**
 * Parse a string into an integer
 *
 * @param s The string to parse.
 * @param out The output integer variable to store the parsed value.
 * @return True if parsing was successful, false otherwise.
 */
bool parse_int(const std::string &s, int &out);

/**
 * Parse a string into a uid_t
 *
 * @param s The string to parse.
 * @param out The output uid_t variable to store the parsed value.
 * @return True if parsing was successful, false otherwise.
 */
bool parse_uid(const std::string &s, uid_t &out);

/**
 * Parse a string into a double
 *
 * @param s The string to parse.
 * @param out The output double variable to store the parsed value.
 * @return True if parsing was successful, false otherwise.
 */
bool parse_double(const std::string &s, double &out);

/**
 * Convert a username into a uid_t
 *
 * @param name The username of the user.
 * @param out The output uid_t variable to store the converted value.
 * @return True if conversion was successful, false otherwise.
 */
bool uid_from_username(const std::string &name, uid_t &out);

/**
 * Convert a uid_t into a username
 *
 * @param uid The uid_t to convert.
 * @param out The output string variable to store the converted value.
 * @return True if conversion was successful, false otherwise.
 */
bool username_from_uid(uid_t uid, std::string &out);
