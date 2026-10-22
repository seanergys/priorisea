#include "src/cli/endpoint/flux/user.hpp"
#include "src/cli/endpoint/util.hpp"
#include "src/lib/error.hpp"
#include "src/lib/user.hpp"
#include "src/lib/util.hpp"
#include <flux/core.h>
#include <jansson.h>
#include <string>
#include <sys/types.h>

ps_result_t<ps_user_t>
ps_cli_endpoint_flux_user_get(const std::string &username) {
  // Resolve the username to a user id locally (getpwnam). The daemon keys
  // purely on the userid, so only the id is sent over the wire.
  uid_t userid;
  if (!uid_from_username(username, userid)) {
    return ps_error(ps_db_not_found_t{.key = "user." + username});
  }

  auto h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }
  auto *f = flux_rpc_pack(h, "priorisea.daemon.user.get", FLUX_NODEID_ANY, 0,
                          "{s:i}", "data", static_cast<int>(userid));
  if (!f) {
    flux_close(h);
    return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
  }

  json_t *user_json;
  if (flux_rpc_get_unpack(f, "{s:o}", "data", &user_json) < 0) {
    auto err = ps_cli_endpoint_unpack_err(f);
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(err);
  }
  auto user = ps_user_from_json(user_json);
  flux_future_destroy(f);
  flux_close(h);
  return user;
}

ps_result_t<std::vector<ps_user_t>>
ps_cli_endpoint_flux_user_list(const std::vector<std::string> &usernames) {
  auto h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }

  // Resolve every username to a user id locally before sending.
  std::vector<uid_t> userids;
  for (const auto &username : usernames) {
    uid_t userid;
    if (!uid_from_username(username, userid)) {
      flux_close(h);
      return ps_error(ps_db_not_found_t{.key = "user." + username});
    }
    userids.push_back(userid);
  }

  flux_future_t *f;
  if (userids.empty()) {
    // No parameter: request the entire list
    // An empty object is sent
    f = flux_rpc_pack(h, "priorisea.daemon.user.list", FLUX_NODEID_ANY, 0,
                      "{}");
  } else {
    json_t *data = json_array();
    for (const auto &userid : userids) {
      json_array_append_new(data,
                            json_integer(static_cast<json_int_t>(userid)));
    }
    f = flux_rpc_pack(h, "priorisea.daemon.user.list", FLUX_NODEID_ANY, 0,
                      "{s:O}", "data", data);
    json_decref(data);
  }
  if (!f) {
    flux_close(h);
    return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
  }

  json_t *users_json;
  if (flux_rpc_get_unpack(f, "{s:o}", "data", &users_json) < 0) {
    auto err = ps_cli_endpoint_unpack_err(f);
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(err);
  }

  std::vector<ps_user_t> users;
  size_t index;
  json_t *value;
  json_array_foreach(users_json, index, value) {
    auto user_res = ps_user_from_json(value);
    if (!ps_is_ok(user_res)) {
      flux_future_destroy(f);
      flux_close(h);
      return ps_forward_error<std::vector<ps_user_t>>(user_res);
    }
    users.push_back(ps_unwrap(user_res));
  }

  flux_future_destroy(f);
  flux_close(h);
  return users;
}

ps_result_t<ps_success_t>
ps_cli_endpoint_flux_user_update(const std::vector<std::string> &usernames,
                                 bool all) {
  auto h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }

  // Resolve every username to a user id locally before sending.
  std::vector<uid_t> userids;
  for (const auto &username : usernames) {
    uid_t userid;
    if (!uid_from_username(username, userid)) {
      flux_close(h);
      return ps_error(ps_db_not_found_t{.key = "user." + username});
    }
    userids.push_back(userid);
  }

  flux_future_t *f;
  if (userids.empty() && all) {
    // An empty object is sent
    f = flux_rpc_pack(h, "priorisea.daemon.user.update", FLUX_NODEID_ANY, 0,
                      "{}");
  } else {
    json_t *data = json_array();
    for (const auto &userid : userids) {
      json_array_append_new(data,
                            json_integer(static_cast<json_int_t>(userid)));
    }
    f = flux_rpc_pack(h, "priorisea.daemon.user.update", FLUX_NODEID_ANY, 0,
                      "{s:O}", "data", data);
    json_decref(data);
  }
  if (!f) {
    flux_close(h);
    return ps_error(ps_flux_rpc_err_t{.strerror = strerror(errno)});
  }

  if (flux_rpc_get(f, NULL) < 0) {
    auto err = std::string(flux_future_error_string(f));
    flux_future_destroy(f);
    flux_close(h);
    return ps_error(ps_daemon_err_t{.err = err});
  }
  flux_future_destroy(f);
  flux_close(h);
  return ps_success_t{};
}
