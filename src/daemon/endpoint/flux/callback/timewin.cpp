// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/daemon/endpoint/flux/callback/timewin.hpp"
#include "src/database/sqlite/timewin.hpp"
#include "src/lib/error.hpp"
#include "src/lib/timewin.hpp"
#include <cerrno>
#include <flux/core.h>
#include <flux/core/response.h>
#include <jansson.h>
#include <string>
#include <vector>

void ps_daemon_endpoint_flux_timewin_get(flux_t *h, const flux_msg_t *msg,
                                         sqlite3 *db) {
  // Retrieve the timewin name from the payload
  const char *name;
  if (flux_request_unpack(msg, NULL, "{s:s}", "data", &name) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
    return;
  }

  // Get the timewin from the database
  auto get_res = ps_db_sqlite_timewin_get(db, name);
  if (!ps_is_ok(get_res)) {
    flux_respond_error(h, msg, 0, ps_error_message(get_res).c_str());
    return;
  }

  // Return the single timewin
  flux_respond_pack(h, msg, "{s:o}", "data",
                    ps_timewin_to_json(ps_unwrap(get_res)));
}

void ps_daemon_endpoint_flux_timewin_list(flux_t *h, const flux_msg_t *msg,
                                          sqlite3 *db) {
  // Retrieve the optional array of timewin names from the payload
  json_t *data_json = nullptr;
  if (flux_request_unpack(msg, NULL, "{s?o}", "data", &data_json) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
    return;
  }

  std::vector<ps_timewin_t> timewins;
  if (data_json == nullptr) {
    // No parameter: return the entire list
    auto list_res = ps_db_sqlite_timewins_list(db);
    if (!ps_is_ok(list_res)) {
      flux_respond_error(h, msg, 0, ps_error_message(list_res).c_str());
      return;
    }
    timewins = ps_unwrap(list_res);
  } else {
    if (!json_is_array(data_json)) {
      auto err = ps_field_type_mismatch_t{.field = "data", .expected = "array"};
      flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
      return;
    }

    // Build the list of timewin names
    std::vector<std::string> names;
    size_t index;
    json_t *value;
    json_array_foreach(data_json, index, value) {
      if (!json_is_string(value)) {
        auto err = ps_field_type_mismatch_t{
            .field = "data[" + std::to_string(index) + "]",
            .expected = "string"};
        flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
        return;
      }
      names.push_back(json_string_value(value));
    }

    // Get the timewins from the database
    auto get_res = ps_db_sqlite_timewins_get(db, names);
    if (!ps_is_ok(get_res)) {
      flux_respond_error(h, msg, 0, ps_error_message(get_res).c_str());
      return;
    }
    auto timewins_map = ps_unwrap(get_res);

    // Build the list of timewins, preserving the request order
    for (const auto &name : names) {
      // The timewin is guaranteed to be in the map
      auto &timewin_res = timewins_map[name];
      if (!ps_is_ok(timewin_res)) {
        flux_respond_error(h, msg, 0, ps_error_message(timewin_res).c_str());
        return;
      }
      timewins.push_back(ps_unwrap(timewin_res));
    }
  }

  // Return as an array of timewins
  json_t *timewins_json = json_array();
  for (const auto &timewin : timewins) {
    json_array_append(timewins_json, ps_timewin_to_json(timewin));
  }
  flux_respond_pack(h, msg, "{s:o}", "data", timewins_json);
}

void ps_daemon_endpoint_flux_timewin_update(flux_t *h, const flux_msg_t *msg,
                                            sqlite3 *db) {
  // Retrieve the timewin from the payload
  json_t *timewin_json;
  if (flux_request_unpack(msg, NULL, "{s:o}", "data", &timewin_json) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
    return;
  }
  auto timewin_res = ps_timewin_from_json(timewin_json);
  if (!ps_is_ok(timewin_res)) {
    flux_respond_error(h, msg, 0, ps_error_message(timewin_res).c_str());
    return;
  }
  auto timewin = ps_unwrap(timewin_res);

  // Validate the timewin
  auto validate_res = ps_validate_timewin(timewin);
  if (!ps_is_ok(validate_res)) {
    flux_respond_error(h, msg, 0, ps_error_message(validate_res).c_str());
    return;
  }

  // Update the timewin in the database (fails if it does not exist)
  auto update_res = ps_db_sqlite_timewin_update(db, timewin);
  if (!ps_is_ok(update_res)) {
    flux_respond_error(h, msg, 0, ps_error_message(update_res).c_str());
    return;
  }
  flux_respond(h, msg, NULL);
}

void ps_daemon_endpoint_flux_timewin_create(flux_t *h, const flux_msg_t *msg,
                                            sqlite3 *db) {
  // Retrieve the timewin from the payload
  json_t *timewin_json;
  if (flux_request_unpack(msg, NULL, "{s:o}", "data", &timewin_json) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
    return;
  }
  auto timewin_res = ps_timewin_from_json(timewin_json);
  if (!ps_is_ok(timewin_res)) {
    flux_respond_error(h, msg, 0, ps_error_message(timewin_res).c_str());
    return;
  }
  auto timewin = ps_unwrap(timewin_res);
  // Validate the timewin
  auto validate_res = ps_validate_timewin(timewin);
  if (!ps_is_ok(validate_res)) {
    flux_respond_error(h, msg, 0, ps_error_message(validate_res).c_str());
    return;
  }
  // Create the timewin in the database (fails if it already exists)
  auto insert_res = ps_db_sqlite_timewin_insert(db, timewin);
  if (!ps_is_ok(insert_res)) {
    flux_respond_error(h, msg, 0, ps_error_message(insert_res).c_str());
    return;
  }
  flux_respond(h, msg, NULL);
}

void ps_daemon_endpoint_flux_timewin_delete(flux_t *h, const flux_msg_t *msg,
                                            sqlite3 *db) {
  // Retrieve the array of timewin names from the payload
  json_t *names_json;
  if (flux_request_unpack(msg, NULL, "{s:o}", "data", &names_json) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
    return;
  }

  if (!json_is_array(names_json)) {
    auto err = ps_field_type_mismatch_t{.field = "data", .expected = "array"};
    flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
    return;
  }

  // Build the list of timewin names
  std::vector<std::string> names;
  size_t index;
  json_t *value;
  json_array_foreach(names_json, index, value) {
    if (!json_is_string(value)) {
      auto err = ps_field_type_mismatch_t{
          .field = "data[" + std::to_string(index) + "]", .expected = "string"};
      flux_respond_error(h, msg, 0, ps_error_message(err).c_str());
      return;
    }
    names.push_back(json_string_value(value));
  }

  // Delete the timewins from the database
  auto del_res = ps_db_sqlite_timewins_del(db, names);
  if (!ps_is_ok(del_res)) {
    flux_respond_error(h, msg, 0, ps_error_message(del_res).c_str());
    return;
  }
  flux_respond(h, msg, NULL);
}
