#include "src/cli/endpoint/flux/user.hpp"
#include "src/cli/endpoint/util.hpp"
#include "src/lib/error.hpp"
#include "src/lib/job.hpp"
#include "src/lib/user.hpp"
#include "src/lib/util.hpp"
#include <flux/core.h>
#include <jansson.h>
#include <string>
#include <sys/types.h>

ps_result_t<ps_cli_endpoint_user_get_t>
ps_cli_endpoint_flux_user_get(const std::string &username, bool plain) {
  // Resolve the username to a user id locally (getpwnam). The daemon keys
  // purely on the userid, so only the id is sent over the wire.
  uid_t userid;
  if (!uid_from_username(username, userid)) {
    return ps_error(ps_db_not_found_t{.key = "user." + username});
  }

  return ps_cli_endpoint_flux_rpc<ps_cli_endpoint_user_get_t>(
      [&](flux_t *h) {
        return flux_rpc_pack(h, "priorisea.daemon.user.get", FLUX_NODEID_ANY, 0,
                             "{s:{s:i s:b}}", "data", "userid",
                             static_cast<int>(userid), "plain", plain);
      },
      [&](flux_future_t *f) -> ps_result_t<ps_cli_endpoint_user_get_t> {
        json_t *user_json;
        json_t *jobs_json;
        if (flux_rpc_get_unpack(f, "{s:{s:o s?o}}", "data", "user", &user_json,
                                "jobs", &jobs_json) < 0) {
          return ps_error(ps_cli_endpoint_unpack_err(f));
        }
        auto user_res = ps_user_from_json(user_json);
        if (!ps_is_ok(user_res)) {
          return ps_forward_error<ps_cli_endpoint_user_get_t>(user_res);
        }
        auto user = ps_unwrap(user_res);
        if (plain) {
          return ps_cli_endpoint_user_get_t{.user = user};
        }

        auto jobs_res = ps_jobs_from_json(jobs_json);
        if (!ps_is_ok(jobs_res)) {
          return ps_forward_error<ps_cli_endpoint_user_get_t>(jobs_res);
        }
        auto jobs = ps_unwrap(jobs_res);
        return ps_cli_endpoint_user_get_t{.user = user, .jobs = jobs};
      });
}

ps_result_t<std::vector<ps_user_t>>
ps_cli_endpoint_flux_user_list(const std::vector<std::string> &usernames,
                               ps_db_order_t order) {
  // Resolve every username to a user id locally before sending.
  std::vector<uid_t> userids;
  for (const auto &username : usernames) {
    uid_t userid;
    if (!uid_from_username(username, userid)) {
      return ps_error(ps_db_not_found_t{.key = "user." + username});
    }
    userids.push_back(userid);
  }

  auto order_str = ps_db_order_to_str(order);
  return ps_cli_endpoint_flux_rpc<std::vector<ps_user_t>>(
      [&](flux_t *h) -> flux_future_t * {
        if (userids.empty()) {
          // No parameter: request the entire list
          return flux_rpc_pack(h, "priorisea.daemon.user.list", FLUX_NODEID_ANY,
                               0, "{s:s}", "order", order_str.c_str());
        }
        json_t *data = json_array();
        for (const auto &userid : userids) {
          json_array_append_new(data,
                                json_integer(static_cast<json_int_t>(userid)));
        }
        auto *f = flux_rpc_pack(h, "priorisea.daemon.user.list",
                                FLUX_NODEID_ANY, 0, "{s:O s:s}", "data", data,
                                "order", order_str.c_str());
        json_decref(data);
        return f;
      },
      [&](flux_future_t *f) -> ps_result_t<std::vector<ps_user_t>> {
        json_t *users_json;
        if (flux_rpc_get_unpack(f, "{s:o}", "data", &users_json) < 0) {
          return ps_error(ps_cli_endpoint_unpack_err(f));
        }
        std::vector<ps_user_t> users;
        size_t index;
        json_t *value;
        json_array_foreach(users_json, index, value) {
          auto user_res = ps_user_from_json(value);
          if (!ps_is_ok(user_res)) {
            return ps_forward_error<std::vector<ps_user_t>>(user_res);
          }
          users.push_back(ps_unwrap(user_res));
        }
        return users;
      });
}

ps_result_t<ps_success_t>
ps_cli_endpoint_flux_user_update(const std::vector<std::string> &usernames,
                                 bool all) {
  // Resolve every username to a user id locally before sending.
  std::vector<uid_t> userids;
  for (const auto &username : usernames) {
    uid_t userid;
    if (!uid_from_username(username, userid)) {
      return ps_error(ps_db_not_found_t{.key = "user." + username});
    }
    userids.push_back(userid);
  }

  return ps_cli_endpoint_flux_rpc<ps_success_t>(
      [&](flux_t *h) -> flux_future_t * {
        if (userids.empty() && all) {
          // An empty object is sent
          return flux_rpc_pack(h, "priorisea.daemon.user.update",
                               FLUX_NODEID_ANY, 0, "{}");
        }
        json_t *data = json_array();
        for (const auto &userid : userids) {
          json_array_append_new(data,
                                json_integer(static_cast<json_int_t>(userid)));
        }
        auto *f = flux_rpc_pack(h, "priorisea.daemon.user.update",
                                FLUX_NODEID_ANY, 0, "{s:O}", "data", data);
        json_decref(data);
        return f;
      },
      [&](flux_future_t *f) -> ps_result_t<ps_success_t> {
        if (flux_rpc_get(f, NULL) < 0) {
          return ps_error(
              ps_daemon_err_t{.err = std::string(flux_future_error_string(f))});
        }
        return ps_success_t{};
      });
}
