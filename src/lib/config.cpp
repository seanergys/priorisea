// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "config.hpp"
#include "error.hpp"
#include <cstdlib>
#include <jansson.h>
#include <optional>
#include <sys/stat.h>
#include <unistd.h>

std::string ps_config_default_path() {
  const char *xdg = std::getenv("XDG_CONFIG_HOME");
  std::string base = xdg && xdg[0] ? xdg : "";
  if (base.empty()) {
    const char *home = std::getenv("HOME");
    if (home)
      base = std::string(home) + "/.config";
  }
  return base + "/priorisea/config.json";
}

ps_result_t<ps_seanerbus_t>
ps_get_seanerbus(const std::optional<json_t *> &config_json) {
  ps_seanerbus_t seanerbus = "demobus://localhost:5398";

  if (config_json.has_value()) {
    json_t *seanerbus_conf = NULL;
    if (json_unpack((config_json.value()), "{s:o}", "seanerbus",
                    &seanerbus_conf) < 0) {
      return ps_error(
          ps_json_unpack_err_t{.dumps = json_dumps(config_json.value(), 0)});
    }
    auto seanerbus_res = ps_seanerbus_from_json(seanerbus_conf);
    if (ps_is_ok(seanerbus_res)) {
      seanerbus = ps_unwrap(seanerbus_res);
    }
  }

  if (const char *url = std::getenv("SEANERBUS_URL")) {
    seanerbus = url;
  }

  return seanerbus;
}

ps_result_t<ps_config_t>
ps_config_load(const std::optional<std::string> &path) {

  std::optional<json_t *> json_opt;

  struct stat st;
  if (path.has_value() && stat(path.value().c_str(), &st) == 0) {
    json_error_t json_err;
    auto *json = json_load_file(path.value().c_str(), 0, &json_err);
    if (!json) {
      return ps_error(ps_config_err_t{.path = path.value(),
                                      .msg = "could not parse JSON: " +
                                             std::string(json_err.text)});
    }
    json_opt = json;
  }

  ps_config_t config;
  auto seanerbus_res = ps_get_seanerbus(json_opt);
  if (!ps_is_ok(seanerbus_res)) {
    return ps_forward_error<ps_config_t>(seanerbus_res);
  }
  config.seanerbus = ps_unwrap(seanerbus_res);
  return config;
}

json_t *ps_config_to_json(const ps_config_t &config) {
  auto *obj = json_object();
  json_object_set_new(obj, "seanerbus", ps_seanerbus_to_json(config.seanerbus));
  return obj;
}

json_t *ps_seanerbus_to_json(const ps_seanerbus_t &seanerbus) {
  return json_string(seanerbus.c_str());
}

ps_result_t<ps_seanerbus_t> ps_seanerbus_from_json(json_t *obj) {
  if (!json_is_string(obj)) {
    return ps_error(ps_seanerbus_url_invalid_t{});
  }
  return json_string_value(obj);
}
