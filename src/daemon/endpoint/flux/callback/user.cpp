// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/daemon/endpoint/flux/callback/user.hpp"
#include "src/database/sqlite/user.hpp"
#include "src/lib/error.hpp"
#include "src/lib/user.hpp"
#include <jansson.h>
#include <string>
#include <sys/types.h>

int ps_daemon_endpoint_flux_user_get(
    const ps_daemon_endpoint_flux_callback_args_t &args) {
  // Retrieve the username from the payload
  const char *username;
  if (flux_request_unpack(args.msg, NULL, "{s:s}", "data", &username) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }

  // Retrieve the user from the database
  auto get_res = ps_db_sqlite_user_get(args.db, username);
  if (!ps_is_ok(get_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(get_res).c_str());
  }
  auto user = ps_unwrap(get_res);

  // Authorize: owners may read any user, others only themselves
  uint32_t userid;
  uint32_t rolemask;
  if (flux_msg_get_userid(args.msg, &userid) < 0 ||
      flux_msg_get_rolemask(args.msg, &rolemask) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }
  if (!(rolemask & FLUX_ROLE_OWNER) &&
      user.userid != static_cast<uid_t>(userid)) {
    auto err = ps_access_denied_t{.key = "user." + std::string(username)};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }

  return flux_respond_pack(args.h, args.msg, "{s:o}", "data",
                           ps_user_to_json(user));
}
int ps_daemon_endpoint_flux_user_list(
    const ps_daemon_endpoint_flux_callback_args_t &args) {
  // Retrieve the optional array of usernames from the payload
  json_t *data_json = nullptr;
  if (flux_request_unpack(args.msg, NULL, "{s?o}", "data", &data_json) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }

  std::vector<ps_user_t> users;

  if (data_json == nullptr) {
    // No parameter: return the entire list
    auto list_res = ps_db_sqlite_users_list(args.db);
    if (!ps_is_ok(list_res)) {
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(list_res).c_str());
    }
    users = ps_unwrap(list_res);
  } else {
    if (!json_is_array(data_json)) {
      auto err = ps_field_type_mismatch_t{.field = "data", .expected = "array"};
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(err).c_str());
    }

    // Build the list of usernames
    std::vector<std::string> usernames;
    size_t index;
    json_t *value;
    json_array_foreach(data_json, index, value) {
      if (!json_is_string(value)) {
        auto err = ps_field_type_mismatch_t{
            .field = "data[" + std::to_string(index) + "]",
            .expected = "string"};
        return flux_respond_error(args.h, args.msg, 0,
                                  ps_error_message(err).c_str());
      }
      usernames.push_back(json_string_value(value));
    }

    // Retrieve the users from the database
    auto get_res = ps_db_sqlite_users_get(args.db, usernames);
    if (!ps_is_ok(get_res)) {
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(get_res).c_str());
    }
    auto users_map = ps_unwrap(get_res);

    // Build the list of users, preserving the request order
    for (const auto &username : usernames) {
      // The user is guaranteed to be in the map
      auto &user_res = users_map[username];
      if (!ps_is_ok(user_res)) {
        return flux_respond_error(args.h, args.msg, 0,
                                  ps_error_message(user_res).c_str());
      }
      users.push_back(ps_unwrap(user_res));
    }
  }

  // Return as an array of users
  json_t *users_json = json_array();
  for (const auto &user : users) {
    json_array_append(users_json, ps_user_to_json(user));
  }
  return flux_respond_pack(args.h, args.msg, "{s:o}", "data", users_json);
}

int ps_daemon_endpoint_flux_user_update(
    const ps_daemon_endpoint_flux_callback_args_t &args) {
  return 0;
}
