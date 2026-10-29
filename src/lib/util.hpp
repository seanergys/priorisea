#pragma once
#include "src/lib/error.hpp"
#include <string>
#include <sys/types.h>

/**
 * Sort order of a listing, ascending or descending
 */
enum class ps_db_order_t {
  asc,
  desc,
};

/**
 * Parse a string into an integer
 *
 * @param s The string to parse
 * @param out The output integer variable to store the parsed value
 * @return True if parsing was successful, false otherwise
 */
bool parse_int(const std::string &s, int &out);

/**
 * Parse a string into a uid_t
 *
 * @param s The string to parse
 * @param out The output uid_t variable to store the parsed value
 * @return True if parsing was successful, false otherwise
 */
bool parse_uid(const std::string &s, uid_t &out);

/**
 * Parse a string into a double
 *
 * @param s The string to parse
 * @param out The output double variable to store the parsed value
 * @return True if parsing was successful, false otherwise
 */
bool parse_double(const std::string &s, double &out);

/**
 * Parse a string into a uint64_t
 *
 * @param s The string to parse
 * @param out The output uint64_t variable to store the parsed value
 * @return True if parsing was successful, false otherwise
 */
bool parse_uint64(const std::string &s, uint64_t &out);

/**
 * Convert a username into a uid_t
 *
 * @param name The username of the user
 * @param out The output uid_t variable to store the converted value
 * @return True if conversion was successful, false otherwise
 */
bool uid_from_username(const std::string &name, uid_t &out);

/**
 * Convert a uid_t into a username
 *
 * @param uid The uid_t to convert
 * @param out The output string variable to store the converted value
 * @return True if conversion was successful, false otherwise
 */
bool username_from_uid(uid_t uid, std::string &out);

/**
 * Convert a sort order to its SQL keyword ("ASC" or "DESC")
 *
 * @param order The sort order to convert
 * @return The corresponding SQL keyword as a string
 */
std::string ps_db_order_to_str(ps_db_order_t order);

/**
 * Convert a sort order string to the enum type
 *
 * @param order The sort order string to convert
 * @return The corresponding enum type or an error if the string is invalid
 */
ps_result_t<ps_db_order_t> ps_db_order_from_str(std::string order);
