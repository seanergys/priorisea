// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/database/sqlite/job.hpp"
#include "src/database/error.hpp"
#include "src/database/sqlite/sqlite.hpp"
#include "src/lib/util.hpp"
#include <cstdint>
#include <src/daemon/endpoint/flux/endpoint.hpp>
#include <string>
#include <sys/types.h>

ps_result_t<ps_job_t> ps_db_sqlite_job_get(sqlite3 *db, flux_jobid_t jobid) {
  auto get_res = ps_db_sqlite_jobs_get(db, {jobid});
  if (!ps_is_ok(get_res)) {
    return ps_forward_error<ps_job_t>(get_res, "Failed to get job");
  }
  auto jobs = ps_unwrap(get_res);
  auto it = jobs.find(jobid);
  if (it == jobs.end()) {
    // ps_db_sqlite_jobs_get returns an entry for every requested jobid
    return ps_error(ps_db_not_found_t{.key = "job." + std::to_string(jobid)});
  }
  return it->second;
}

ps_result_t<ps_db_sqlite_jobs_get_t>
ps_db_sqlite_jobs_get(sqlite3 *db, const std::vector<flux_jobid_t> &jobids) {
  std::string placeholders;
  for (size_t i = 0; i < jobids.size(); ++i) {
    if (i > 0)
      placeholders += ", ";
    placeholders += "?";
  }
  auto sql = "SELECT id, userid, t_inactive, score "
             "FROM jobs "
             "WHERE id IN (" +
             placeholders + ");";

  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }

  // Bind the jobids
  for (auto i = 0; i < jobids.size(); i++) {
    if (sqlite3_bind_text(stmt, i + 1, std::to_string(jobids[i]).c_str(), -1,
                          SQLITE_TRANSIENT) != SQLITE_OK) {
      sqlite3_finalize(stmt);
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
  }

  ps_db_sqlite_jobs_get_t res;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    auto id_str = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
    uint64_t id;
    if (!parse_uint64(id_str, id)) {
      // If the id cannot be parsed, skip this row
      continue;
    }
    res[id] =
        ps_job_t{.jobid = id_str,
                 .userid = static_cast<uid_t>(sqlite3_column_int(stmt, 1)),
                 .t_inactive = sqlite3_column_double(stmt, 2),
                 .score = sqlite3_column_double(stmt, 3)};
  }
  sqlite3_finalize(stmt);

  // Add errors for jobs that were not found
  for (const auto &jobid : jobids) {
    if (res.find(jobid) == res.end()) {
      res[jobid] =
          ps_error(ps_db_not_found_t{.key = "job." + std::to_string(jobid)});
    }
  }

  return res;
}

ps_result_t<std::vector<ps_job_t>> ps_db_sqlite_jobs_list(sqlite3 *db,
                                                          size_t limit) {
  // Most recent first: higher t_inactive sorts earlier.
  auto sql = "SELECT id, userid, t_inactive, score "
             "FROM jobs "
             "ORDER BY t_inactive DESC "
             "LIMIT ?;";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  // Bind the limit
  if (sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(limit)) !=
      SQLITE_OK) {
    sqlite3_finalize(stmt);
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }

  std::vector<ps_job_t> res;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    auto id_str = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
    res.push_back(ps_job_t{
        .jobid = id_str ? id_str : "",
        .userid = static_cast<uid_t>(sqlite3_column_int(stmt, 1)),
        .t_inactive = sqlite3_column_double(stmt, 2),
        .score = sqlite3_column_double(stmt, 3),
    });
  }
  sqlite3_finalize(stmt);
  return res;
}

ps_result_t<std::vector<ps_job_t>>
ps_db_sqlite_jobs_list(sqlite3 *db, size_t limit, uid_t userid) {
  // Most recent first: higher t_inactive sorts earlier.
  auto sql = "SELECT id, userid, t_inactive, score "
             "FROM jobs "
             "WHERE userid = ? "
             "ORDER BY t_inactive DESC "
             "LIMIT ?;";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  // Bind the parameters
  if (sqlite3_bind_int(stmt, 1, userid) != SQLITE_OK ||
      sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(limit)) !=
          SQLITE_OK) {
    sqlite3_finalize(stmt);
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }

  std::vector<ps_job_t> res;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    auto id_str = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
    res.push_back(ps_job_t{
        .jobid = id_str ? id_str : "",
        .userid = static_cast<uid_t>(sqlite3_column_int(stmt, 1)),
        .t_inactive = sqlite3_column_double(stmt, 2),
        .score = sqlite3_column_double(stmt, 3),
    });
  }
  sqlite3_finalize(stmt);
  return res;
}

ps_result_t<std::vector<ps_job_t>>
ps_db_sqlite_jobs_list(sqlite3 *db, double min_t_inactive) {
  // Most recent first: higher t_inactive sorts earlier.
  auto sql = "SELECT id, userid, t_inactive, score "
             "FROM jobs "
             "WHERE t_inactive >= ? "
             "ORDER BY t_inactive DESC;";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  // Bind t_inactive
  if (sqlite3_bind_double(stmt, 1, min_t_inactive) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }

  std::vector<ps_job_t> res;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    auto id_str = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
    res.push_back(ps_job_t{
        .jobid = id_str ? id_str : "",
        .userid = static_cast<uid_t>(sqlite3_column_int(stmt, 1)),
        .t_inactive = sqlite3_column_double(stmt, 2),
        .score = sqlite3_column_double(stmt, 3),
    });
  }
  sqlite3_finalize(stmt);
  return res;
}

ps_result_t<std::vector<ps_job_t>>
ps_db_sqlite_jobs_list(sqlite3 *db, double min_t_inactive,
                       const std::vector<uid_t> &userids) {
  // Most recent first: higher t_inactive sorts earlier.
  std::vector<ps_job_t> res;

  // An empty user list can never match any job; skip building an invalid
  // "userid IN ()" clause.
  if (userids.empty()) {
    return res;
  }

  std::string placeholders;
  for (size_t i = 0; i < userids.size(); ++i) {
    if (i > 0)
      placeholders += ", ";
    placeholders += "?";
  }
  auto sql = "SELECT id, userid, t_inactive, score "
             "FROM jobs "
             "WHERE t_inactive >= ? AND userid IN (" +
             placeholders +
             ") "
             "ORDER BY t_inactive DESC;";

  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }

  // Bind t_inactive first (param 1), then each user id (params 2..n+1).
  if (sqlite3_bind_double(stmt, 1, min_t_inactive) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  for (size_t i = 0; i < userids.size(); i++) {
    if (sqlite3_bind_int(stmt, static_cast<int>(i + 2), userids[i]) !=
        SQLITE_OK) {
      sqlite3_finalize(stmt);
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    auto id_str = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
    res.push_back(ps_job_t{
        .jobid = id_str ? id_str : "",
        .userid = static_cast<uid_t>(sqlite3_column_int(stmt, 1)),
        .t_inactive = sqlite3_column_double(stmt, 2),
        .score = sqlite3_column_double(stmt, 3),
    });
  }
  sqlite3_finalize(stmt);
  return res;
}

ps_result_t<ps_success_t>
ps_db_sqlite_jobs_set(sqlite3 *db, const std::vector<ps_job_t> &jobs) {
  if (jobs.empty()) {
    return ps_success_t{};
  }

  auto rollback = [&] {
    sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
  };

  if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = "BEGIN", .message = sqlite3_errmsg(db)});
  }

  auto sql = "INSERT OR REPLACE INTO jobs (id, userid, t_inactive, score) "
             "VALUES (?, ?, ?, ?);";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  for (const auto &job : jobs) {
    if (sqlite3_bind_text(stmt, 1, job.jobid.c_str(), -1, SQLITE_TRANSIENT) !=
            SQLITE_OK ||
        sqlite3_bind_int(stmt, 2, job.userid) != SQLITE_OK ||
        sqlite3_bind_double(stmt, 3, job.t_inactive) != SQLITE_OK ||
        sqlite3_bind_double(stmt, 4, job.score) != SQLITE_OK) {
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
  }
  sqlite3_finalize(stmt);

  if (sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = "COMMIT", .message = sqlite3_errmsg(db)});
  }

  return ps_success_t{};
}

ps_result_t<ps_success_t>
ps_db_sqlite_jobs_set(sqlite3 *db, const std::vector<ps_job_t> &jobs,
                      double t_inactive) {
  auto rollback = [&] {
    sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
  };

  if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = "BEGIN", .message = sqlite3_errmsg(db)});
  }

  auto sql = "INSERT OR REPLACE INTO jobs (id, userid, t_inactive, score) "
             "VALUES (?, ?, ?, ?);";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  for (const auto &job : jobs) {
    if (sqlite3_bind_text(stmt, 1, job.jobid.c_str(), -1, SQLITE_TRANSIENT) !=
            SQLITE_OK ||
        sqlite3_bind_int(stmt, 2, job.userid) != SQLITE_OK ||
        sqlite3_bind_double(stmt, 3, job.t_inactive) != SQLITE_OK ||
        sqlite3_bind_double(stmt, 4, job.score) != SQLITE_OK) {
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
  }
  sqlite3_finalize(stmt);

  // Advance the t_inactive cursor in the same transaction as the jobs, so the
  // two can never diverge: if the cursor write fails the job writes roll back,
  // and if a job write fails the cursor is never advanced. The UPDATE touches
  // only t_inactive, leaving the NOT NULL user_period untouched.
  auto info_sql = "UPDATE database_info SET t_inactive = ? WHERE id = 1;";
  sqlite3_stmt *info_stmt = nullptr;
  if (sqlite3_prepare_v2(db, info_sql, -1, &info_stmt, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = info_sql, .message = sqlite3_errmsg(db)});
  }
  if (sqlite3_bind_double(info_stmt, 1, t_inactive) != SQLITE_OK) {
    sqlite3_finalize(info_stmt);
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = info_sql, .message = sqlite3_errmsg(db)});
  }
  if (sqlite3_step(info_stmt) != SQLITE_DONE) {
    sqlite3_finalize(info_stmt);
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = info_sql, .message = sqlite3_errmsg(db)});
  }
  sqlite3_finalize(info_stmt);

  if (sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = "COMMIT", .message = sqlite3_errmsg(db)});
  }

  return ps_success_t{};
}

ps_result_t<ps_success_t> ps_db_sqlite_job_set(sqlite3 *db,
                                               const ps_job_t &job) {
  return ps_db_sqlite_jobs_set(db, {job});
}

ps_result_t<ps_success_t>
ps_db_sqlite_jobs_insert(sqlite3 *db, const std::vector<ps_job_t> &jobs) {
  if (jobs.empty()) {
    return ps_success_t{};
  }

  auto rollback = [&] {
    sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
  };

  if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = "BEGIN", .message = sqlite3_errmsg(db)});
  }

  auto sql =
      "INSERT INTO jobs (id, userid, t_inactive, score) VALUES (?, ?, ?, ?);";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  for (const auto &job : jobs) {
    if (sqlite3_bind_text(stmt, 1, job.jobid.c_str(), -1, SQLITE_TRANSIENT) !=
            SQLITE_OK ||
        sqlite3_bind_int(stmt, 2, job.userid) != SQLITE_OK ||
        sqlite3_bind_double(stmt, 3, job.t_inactive) != SQLITE_OK ||
        sqlite3_bind_double(stmt, 4, job.score) != SQLITE_OK) {
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      sqlite3_finalize(stmt);
      rollback();
      if (ps_db_sqlite_is_constraint(db)) {
        return ps_error(ps_db_already_exist_err_t{.key = "job." + job.jobid});
      }
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
  }
  sqlite3_finalize(stmt);

  if (sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = "COMMIT", .message = sqlite3_errmsg(db)});
  }

  return ps_success_t{};
}

ps_result_t<ps_success_t> ps_db_sqlite_job_insert(sqlite3 *db,
                                                  const ps_job_t &job) {
  return ps_db_sqlite_jobs_insert(db, {job});
}

ps_result_t<ps_success_t>
ps_db_sqlite_jobs_update(sqlite3 *db, const std::vector<ps_job_t> &jobs) {
  if (jobs.empty()) {
    return ps_success_t{};
  }

  auto rollback = [&] {
    sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
  };

  if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = "BEGIN", .message = sqlite3_errmsg(db)});
  }

  auto sql =
      "UPDATE jobs SET userid = ?, t_inactive = ?, score = ? WHERE id = ?;";
  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  for (const auto &job : jobs) {
    if (sqlite3_bind_int(stmt, 1, job.userid) != SQLITE_OK ||
        sqlite3_bind_double(stmt, 2, job.t_inactive) != SQLITE_OK ||
        sqlite3_bind_double(stmt, 3, job.score) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 4, job.jobid.c_str(), -1, SQLITE_TRANSIENT) !=
            SQLITE_OK) {
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(
          ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
    }
    if (sqlite3_changes(db) == 0) {
      sqlite3_finalize(stmt);
      rollback();
      return ps_error(ps_db_not_found_t{.key = "job." + job.jobid});
    }
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
  }
  sqlite3_finalize(stmt);

  if (sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr) != SQLITE_OK) {
    rollback();
    return ps_error(
        ps_db_query_err_t{.query = "COMMIT", .message = sqlite3_errmsg(db)});
  }

  return ps_success_t{};
}

ps_result_t<ps_success_t> ps_db_sqlite_job_update(sqlite3 *db,
                                                  const ps_job_t &job) {
  return ps_db_sqlite_jobs_update(db, {job});
}
