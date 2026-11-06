#include "src/database/sqlite/sqlite.hpp"
#include "src/lib/error.hpp"
#include <cerrno>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>

// Creates every parent directory of `path` (mkdir -p semantics).
static ps_result_t<ps_success_t> ps_mkdir_parents(const std::string &path) {
  const std::size_t last_slash = path.find_last_of('/');
  if (last_slash == std::string::npos) {
    // Parent directory already exists
    return ps_success_t{};
  }
  const std::string dir = path.substr(0, last_slash);
  if (dir.empty()) {
    // "/" directory already exists
    return ps_success_t{};
  }
  std::string current;
  for (std::size_t i = 0; i < dir.size(); ++i) {
    current.push_back(dir[i]);
    if (dir[i] == '/' && mkdir(current.c_str(), 0755) != 0 && errno != EEXIST) {
      return ps_error(
          ps_permissions_err_t{.msg = "failed to create directory '" + current +
                                      "': " + std::strerror(errno)});
    }
  }
  if (mkdir(dir.c_str(), 0755) != 0 && errno != EEXIST) {
    return ps_error(ps_permissions_err_t{.msg = "failed to create directory '" +
                                                dir +
                                                "': " + std::strerror(errno)});
  }
  return ps_success_t{};
}

ps_result_t<ps_success_t> ps_db_sqlite_execute(sqlite3 *db,
                                               const std::string &sql) {
  char *sql_err = nullptr;
  int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &sql_err);
  if (rc != SQLITE_OK) {
    std::string str_err = sql_err ? std::string(sql_err) : "Unknown error";
    auto err = ps_error(ps_db_query_err_t{.query = sql, .message = str_err});
    sqlite3_free(sql_err);
    return err;
  }
  return ps_success_t{};
}

// Opens a SQLite database at the given path using the specified open flags.
// The database handle is always released on error to avoid leaking it.
static ps_result_t<sqlite3 *>
ps_db_sqlite_open_with_flags(const std::string &path, int flags) {
  sqlite3 *db = nullptr;
  auto rc = sqlite3_open_v2(path.c_str(), &db, flags, nullptr);
  if (rc != SQLITE_OK) {
    auto err =
        ps_error(ps_db_open_err_t{.path = path, .message = sqlite3_errmsg(db)});
    if (db != nullptr) {
      sqlite3_close(db);
    }
    return err;
  }
  return db;
}

ps_result_t<sqlite3 *> ps_db_sqlite_open(const std::string &path) {
  // Open an existing database only; never create the file
  return ps_db_sqlite_open_with_flags(path, SQLITE_OPEN_READWRITE);
}

ps_result_t<sqlite3 *> ps_db_sqlite_create(const std::string &path) {
  struct stat st;
  if (stat(path.c_str(), &st) == 0) {
    return ps_error(ps_db_already_exist_err_t{.key = path});
  }
  auto mkdir_res = ps_mkdir_parents(path);
  if (!ps_is_ok(mkdir_res)) {
    return ps_forward_error<sqlite3 *>(mkdir_res);
  }
  // Create the database file if it does not exist yet
  return ps_db_sqlite_open_with_flags(path, SQLITE_OPEN_READWRITE |
                                                SQLITE_OPEN_CREATE);
}

ps_result_t<ps_success_t> ps_db_sqlite_create_schema(sqlite3 *db) {
  // job id is stored as TEXT because its type is uint64_t in Flux
  auto sql = "PRAGMA user_version = " + std::to_string(SQLITE_DB_VERSION) +
             ";" +
             R"(
    CREATE TABLE IF NOT EXISTS database_info (
        id INTEGER PRIMARY KEY CHECK(id = 1),
        t_inactive REAL NOT NULL,
        user_period REAL NOT NULL
    );

    CREATE TABLE IF NOT EXISTS users (
        userid INTEGER PRIMARY KEY,
        score REAL,
        w_expr TEXT NOT NULL,
        w_defaults TEXT,
        t_executed REAL NOT NULL,
        t_inactive_min REAL NOT NULL
    );

    CREATE TABLE IF NOT EXISTS jobs (
        id TEXT PRIMARY KEY,
        userid INTEGER NOT NULL,
        t_inactive REAL NOT NULL,
        score REAL NOT NULL,
        duration REAL NOT NULL,
        s_expr TEXT NOT NULL,
        s_defaults TEXT,
        s_values TEXT,
        weight REAL,
        w_values TEXT
    );

    CREATE TABLE IF NOT EXISTS formulas (
        name TEXT PRIMARY KEY,
        expression TEXT NOT NULL,
        defaults TEXT
    );

    CREATE TABLE IF NOT EXISTS timewins (
        name TEXT PRIMARY KEY,
        cron TEXT NOT NULL,
        duration INTEGER NOT NULL,
        score REAL NOT NULL
    );
    )";

  return ps_db_sqlite_execute(db, sql);
}

ps_result_t<int> ps_db_sqlite_user_version_get(sqlite3 *db) {
  sqlite3_stmt *stmt = nullptr;
  auto sql = "PRAGMA user_version;";
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = sqlite3_errmsg(db)});
  }
  if (sqlite3_step(stmt) != SQLITE_ROW) {
    sqlite3_finalize(stmt);
    return ps_error(
        ps_db_query_err_t{.query = sql, .message = "no user_version row"});
  }
  int version = sqlite3_column_int(stmt, 0);
  sqlite3_finalize(stmt);
  return version;
}

ps_result_t<ps_success_t> ps_db_sqlite_user_version_set(sqlite3 *db,
                                                        int version) {
  auto sql = "PRAGMA user_version = " + std::to_string(version) + ";";
  auto res = ps_db_sqlite_execute(db, sql);
  return res;
}

ps_result_t<ps_success_t> ps_db_sqlite_verify(sqlite3 *db) {
  // The database should contain the user_version
  auto version_res = ps_db_sqlite_user_version_get(db);
  if (!ps_is_ok(version_res)) {
    sqlite3_close(db);
    return ps_forward_error<ps_success_t>(version_res);
  }
  // The user_version should match the expected version
  auto version = ps_unwrap(version_res);
  if (version != SQLITE_DB_VERSION) {
    sqlite3_close(db);
    return ps_error(ps_db_version_mismatch_t{.expected = SQLITE_DB_VERSION,
                                             .actual = version});
  }
  return ps_success_t{};
}

bool ps_db_sqlite_is_constraint(sqlite3 *db) {
  return sqlite3_errcode(db) == SQLITE_CONSTRAINT;
}
