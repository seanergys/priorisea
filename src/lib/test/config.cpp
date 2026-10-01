// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "lib/config.hpp"

#include <catch2/catch.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>

TEST_CASE("ps_get_seanerbus accepts well-formed input",
          "[config][ps_get_seanerbus]") {
  // Clear for deterministic default-value assertion.
  unsetenv("SEANERBUS_URL");

  // No configuration provided: the built-in default is used.
  std::optional<json_t *> no_conf;
  auto res = ps_get_seanerbus(no_conf);
  REQUIRE(ps_is_ok(res));
  REQUIRE(ps_unwrap(res) == "demobus://localhost:5398");

  // A configuration carrying a valid seanerbus string overrides the default.
  json_t *obj = json_pack("{s:s}", "seanerbus", "demobus://example:1234");
  std::optional<json_t *> conf = obj;
  auto res2 = ps_get_seanerbus(conf);
  REQUIRE(ps_is_ok(res2));
  REQUIRE(ps_unwrap(res2) == "demobus://example:1234");
  json_decref(obj);
}

TEST_CASE("ps_get_seanerbus rejects malformed input",
          "[config][ps_get_seanerbus]") {
  unsetenv("SEANERBUS_URL");

  // The seanerbus key is missing: json_unpack fails.
  json_t *obj1 = json_object();
  std::optional<json_t *> conf1 = obj1;
  auto res1 = ps_get_seanerbus(conf1);
  REQUIRE_FALSE(ps_is_ok(res1));
  REQUIRE(ps_is_error_type<ps_json_unpack_err_t>(res1));
  json_decref(obj1);

  // The configuration is not a JSON object: json_unpack fails.
  json_t *obj2 = json_array();
  std::optional<json_t *> conf2 = obj2;
  auto res2 = ps_get_seanerbus(conf2);
  REQUIRE_FALSE(ps_is_ok(res2));
  REQUIRE(ps_is_error_type<ps_json_unpack_err_t>(res2));
  json_decref(obj2);
}

TEST_CASE("ps_config_load accepts well-formed input",
          "[config][ps_config_load]") {
  unsetenv("SEANERBUS_URL");

  // No path: default configuration.
  auto res = ps_config_load(std::nullopt);
  REQUIRE(ps_is_ok(res));
  REQUIRE(ps_unwrap(res).seanerbus == "demobus://localhost:5398");

  // A valid configuration file is parsed.
  auto path =
      std::filesystem::temp_directory_path() / "priorisea_test_config.json";
  {
    std::ofstream f(path);
    f << R"({"seanerbus": "demobus://fromfile:5398"})";
  }
  auto res2 = ps_config_load(std::string(path));
  REQUIRE(ps_is_ok(res2));
  REQUIRE(ps_unwrap(res2).seanerbus == "demobus://fromfile:5398");
  std::filesystem::remove(path);
}

TEST_CASE("ps_config_load rejects malformed input",
          "[config][ps_config_load]") {
  auto path =
      std::filesystem::temp_directory_path() / "priorisea_test_bad_config.json";
  {
    std::ofstream f(path);
    f << "{ this is not valid json ";
  }
  auto res = ps_config_load(std::string(path));
  REQUIRE_FALSE(ps_is_ok(res));
  REQUIRE(ps_is_error_type<ps_config_err_t>(res));
  std::filesystem::remove(path);
}

TEST_CASE("ps_seanerbus_from_json accepts well-formed input",
          "[config][ps_seanerbus_from_json]") {
  json_t *s = json_string("demobus://localhost:5398");
  auto res = ps_seanerbus_from_json(s);
  REQUIRE(ps_is_ok(res));
  REQUIRE(ps_unwrap(res) == "demobus://localhost:5398");
  json_decref(s);
}

TEST_CASE("ps_seanerbus_from_json rejects malformed input",
          "[config][ps_seanerbus_from_json]") {
  // A JSON object is not a valid seanerbus URL.
  json_t *o = json_object();
  auto res = ps_seanerbus_from_json(o);
  REQUIRE_FALSE(ps_is_ok(res));
  REQUIRE(ps_is_error_type<ps_seanerbus_url_invalid_t>(res));
  json_decref(o);

  // An integer is not a valid seanerbus URL either.
  json_t *i = json_integer(42);
  auto res2 = ps_seanerbus_from_json(i);
  REQUIRE_FALSE(ps_is_ok(res2));
  REQUIRE(ps_is_error_type<ps_seanerbus_url_invalid_t>(res2));
  json_decref(i);
}

TEST_CASE("ps_seanerbus_to_json and ps_seanerbus_from_json are inverse",
          "[config]") {
  ps_seanerbus_t seanerbus = "demobus://localhost:5398";
  json_t *j = ps_seanerbus_to_json(seanerbus);
  REQUIRE(json_is_string(j));
  auto res = ps_seanerbus_from_json(j);
  REQUIRE(ps_is_ok(res));
  REQUIRE(ps_unwrap(res) == seanerbus);
  json_decref(j);
}
