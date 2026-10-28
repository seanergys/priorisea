#include "src/daemon/endpoint/flux/callback/user.hpp"
#include "src/database/sqlite/formula.hpp"
#include "src/database/sqlite/info.hpp"
#include "src/database/sqlite/job.hpp"
#include "src/database/sqlite/user.hpp"
#include "src/lib/error.hpp"
#include "src/lib/formula.hpp"
#include "src/lib/types.hpp"
#include "src/lib/user.hpp"
#include <chrono>
#include <climits>
#include <cmath>
#include <jansson.h>
#include <string>
#include <sys/types.h>
#include <utility>

// Builds the list of users described by a request payload. When data_json is
// null the entire user list is returned; otherwise data_json must be a JSON
// array of user ids (integers) and the matching users are returned in request
// order. When create_missing is true, user ids that are not yet stored in the
// database are returned as freshly created users (with the default score)
// instead of failing the request.
static ps_result_t<std::vector<ps_user_t>>
ps_collect_users(sqlite3 *db, json_t *data_json, bool create_missing = false) {
  std::vector<ps_user_t> users;

  if (data_json == nullptr) {
    // No parameter: return the entire list
    auto list_res = ps_db_sqlite_users_list(db);
    if (!ps_is_ok(list_res)) {
      return list_res;
    }
    users = ps_unwrap(list_res);
  } else {
    if (!json_is_array(data_json)) {
      return ps_error(
          ps_field_type_mismatch_t{.field = "data", .expected = "array"});
    }

    // Build the list of user ids
    std::vector<uid_t> userids;
    size_t index;
    json_t *value;
    json_array_foreach(data_json, index, value) {
      if (!json_is_integer(value)) {
        return ps_error(ps_field_type_mismatch_t{
            .field = "data[" + std::to_string(index) + "]",
            .expected = "integer"});
      }
      userids.push_back(static_cast<uid_t>(json_integer_value(value)));
    }

    // Retrieve the users from the database
    auto get_res = ps_db_sqlite_users_get(db, userids);
    if (!ps_is_ok(get_res)) {
      return ps_forward_error<std::vector<ps_user_t>>(get_res);
    }
    auto users_map = ps_unwrap(get_res);

    // Build the list of users, preserving the request order
    for (const auto &userid : userids) {
      // The user is guaranteed to be in the map
      auto &user_res = users_map[userid];
      if (!ps_is_ok(user_res)) {
        // A requested user may not exist yet: create it with the default score
        // when allowed, otherwise fail the request
        if (!create_missing) {
          return ps_forward_error<std::vector<ps_user_t>>(user_res);
        }
        users.push_back({.userid = userid, .has_score = false});
        continue;
      }
      users.push_back(ps_unwrap(user_res));
    }
  }
  return users;
}

/**
 * Computes the score for each user based on the given formula and jobs.
 * A user that ran of of the jobs may not appear in the return value
 * if the sum of its weight is 0
 *
 * @param formula The formula to use for computing the score.
 * @param jobs The jobs to use for computing the score.
 * @param max_duration_by_user For each user, the longest job duration among
 *        their jobs, used to weight each job's score by its duration.
 * @return A map of user IDs and their computed scores.
 */
static ps_result_t<std::map<uid_t, double>>
ps_compute_users_score(const ps_formula_t &formula, std::vector<ps_job_t> jobs,
                       const std::map<uid_t, double> &max_duration_by_user) {
  std::map<uid_t, std::pair<double, double>> user_scores;
  auto now = std::chrono::system_clock::now();
  auto now_ts = std::chrono::duration<double>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
  for (const auto &job : jobs) {
    double age_seconds = now_ts - job.t_inactive;
    double age_days = age_seconds / (24.0 * 60.0 * 60.0);
    double duration_seconds = job.duration;
    double duration_hours = duration_seconds / 3600.0;

    // The user's longest job duration in the window (0 when the user has no
    // job in the window), exposed alongside its value in hours for symmetry
    // with the per-job duration variables.
    auto max_duration_seconds = 0.0;
    auto md_it = max_duration_by_user.find(job.userid);
    if (md_it != max_duration_by_user.end()) {
      max_duration_seconds = md_it->second;
    }
    double max_duration_hours = max_duration_seconds / 3600.0;

    std::map<std::string, double> vars_over = {
        {"var:age_seconds", age_seconds},
        {"var:age_days", age_days},
        {"var:duration_seconds", duration_seconds},
        {"var:duration_hours", duration_hours},
        {"var:max_duration_seconds", max_duration_seconds},
        {"var:max_duration_hours", max_duration_hours}};
    auto weight_res = ps_eval_formula(formula, vars_over);
    if (!ps_is_ok(weight_res)) {
      return ps_forward_error<std::map<uid_t, double>>(weight_res);
    }
    auto weight = ps_unwrap(weight_res);

    auto it = user_scores.find(job.userid);
    if (it == user_scores.end()) {
      user_scores[job.userid] = {job.score * weight, weight};
    } else {
      auto old = it->second;
      user_scores[job.userid] = {job.score * weight + old.first,
                                 weight + old.second};
    }
  }

  std::map<uid_t, double> res;
  for (const auto &[user, pair] : user_scores) {
    if (pair.second != 0) {
      auto score = pair.first / pair.second;
      if (!std::isinf(score)) {
        res[user] = score;
      }
    }
  }
  return res;
}

int ps_daemon_endpoint_flux_user_get(
    const ps_daemon_endpoint_flux_callback_args_t &args) {
  // Retrieve the user id from the payload
  int userid_in;
  if (flux_request_unpack(args.msg, NULL, "{s:i}", "data", &userid_in) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }
  auto userid = static_cast<uid_t>(userid_in);

  // Retrieve the user from the database
  auto get_res = ps_db_sqlite_user_get(args.db, userid);
  if (!ps_is_ok(get_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(get_res).c_str());
  }
  auto user = ps_unwrap(get_res);

  // Authorize: owners may read any user, others only themselves
  uint32_t caller_userid;
  uint32_t rolemask;
  if (flux_msg_get_userid(args.msg, &caller_userid) < 0 ||
      flux_msg_get_rolemask(args.msg, &rolemask) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }
  if (!(rolemask & FLUX_ROLE_OWNER) &&
      user.userid != static_cast<uid_t>(caller_userid)) {
    auto err = ps_access_denied_t{.key = "user." + std::to_string(userid)};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }

  return flux_respond_pack(args.h, args.msg, "{s:o}", "data",
                           ps_user_to_json(user));
}

int ps_daemon_endpoint_flux_user_list(
    const ps_daemon_endpoint_flux_callback_args_t &args) {
  // Retrieve the optional array of user ids from the payload
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

    // Build the list of user ids
    std::vector<uid_t> userids;
    size_t index;
    json_t *value;
    json_array_foreach(data_json, index, value) {
      if (!json_is_integer(value)) {
        auto err = ps_field_type_mismatch_t{
            .field = "data[" + std::to_string(index) + "]",
            .expected = "integer"};
        return flux_respond_error(args.h, args.msg, 0,
                                  ps_error_message(err).c_str());
      }
      userids.push_back(static_cast<uid_t>(json_integer_value(value)));
    }

    // Retrieve the users from the database
    auto get_res = ps_db_sqlite_users_get(args.db, userids);
    if (!ps_is_ok(get_res)) {
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(get_res).c_str());
    }
    auto users_map = ps_unwrap(get_res);

    // Build the list of users, preserving the request order
    for (const auto &userid : userids) {
      // The user is guaranteed to be in the map
      auto &user_res = users_map[userid];
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
  // Retrieve the optional array of user ids from the payload
  json_t *data_json = nullptr;
  if (flux_request_unpack(args.msg, NULL, "{s?o}", "data", &data_json) < 0) {
    auto err = ps_flux_rpc_err_t{.strerror = strerror(errno)};
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(err).c_str());
  }

  auto formula_res = ps_db_sqlite_formula_get(args.db, "job_weight");
  if (!ps_is_ok(formula_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(formula_res).c_str());
  }
  auto formula = ps_unwrap(formula_res);

  auto info_res = ps_db_sqlite_info_get(args.db);
  if (!ps_is_ok(info_res)) {
    return flux_respond_error(args.h, args.msg, 0,
                              ps_error_message(info_res).c_str());
  }

  auto now = std::chrono::system_clock::now();
  auto now_ts = std::chrono::duration<double>(now.time_since_epoch()).count();
  auto t_inactive_min = now_ts - ps_unwrap(info_res).user_period;

  if (data_json == nullptr) {
    // Retrieve all jobs
    // Retrieve all users
    // Set all users that appear in the jobs
    // Set all users that appear in the users
    auto jobs_res = ps_db_sqlite_jobs_list(args.db, t_inactive_min);
    if (!ps_is_ok(jobs_res)) {
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(jobs_res).c_str());
    }
    auto jobs_list = ps_unwrap(jobs_res);

    auto users_res = ps_db_sqlite_users_list(args.db);
    if (!ps_is_ok(users_res)) {
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(users_res).c_str());
    }
    auto users = ps_unwrap(users_res);

    auto scores_res = ps_compute_users_score(formula, jobs_list.jobs,
                                             jobs_list.max_duration_by_user);
    if (!ps_is_ok(scores_res)) {
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(scores_res).c_str());
    }
    auto scores = ps_unwrap(scores_res);

    std::vector<ps_user_t> updated;
    for (const auto &user : users) {
      // Users who did not compute a user score
      if (scores.find(user.userid) == scores.end()) {
        updated.push_back({.userid = user.userid, .has_score = false});
      }
    }

    for (const auto &[user, score] : scores) {
      // Users who ran a job are updated
      updated.push_back({.userid = user, .has_score = true, .score = score});
    }

    // Update every new and existing users
    auto update_res = ps_db_sqlite_users_set(args.db, updated);
    if (!ps_is_ok(update_res)) {
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(update_res).c_str());
    }
  } else {
    if (!json_is_array(data_json)) {
      auto err = ps_field_type_mismatch_t{.field = "data", .expected = "array"};
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(err).c_str());
    }

    // Build the list of user ids
    std::vector<uid_t> userids;
    size_t index;
    json_t *value;
    json_array_foreach(data_json, index, value) {
      if (!json_is_integer(value)) {
        auto err = ps_field_type_mismatch_t{
            .field = "data[" + std::to_string(index) + "]",
            .expected = "integer"};
        return flux_respond_error(args.h, args.msg, 0,
                                  ps_error_message(err).c_str());
      }
      userids.push_back(static_cast<uid_t>(json_integer_value(value)));
    }

    // Retrieve the jobs of users passed as arguments
    auto jobs_res = ps_db_sqlite_jobs_list(args.db, t_inactive_min, userids);
    if (!ps_is_ok(jobs_res)) {
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(jobs_res).c_str());
    }
    auto jobs_list = ps_unwrap(jobs_res);

    auto scores_res = ps_compute_users_score(formula, jobs_list.jobs,
                                             jobs_list.max_duration_by_user);
    if (!ps_is_ok(scores_res)) {
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(scores_res).c_str());
    }
    auto scores = ps_unwrap(scores_res);

    // A user may not appear in the list of jobs
    // Their score is unset
    std::vector<ps_user_t> users;
    for (auto &userid : userids) {
      ps_user_t user = {.userid = userid};
      auto it = scores.find(userid);
      if (it == scores.end()) {
        user.has_score = false;
      } else {
        user.has_score = true;
        user.score = it->second;
      }
      users.push_back(user);
    }

    auto update_res = ps_db_sqlite_users_set(args.db, users);
    if (!ps_is_ok(update_res)) {
      return flux_respond_error(args.h, args.msg, 0,
                                ps_error_message(update_res).c_str());
    }
  }
  return flux_respond(args.h, args.msg, NULL);
}
