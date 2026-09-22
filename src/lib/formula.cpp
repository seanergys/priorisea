// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "formula.hpp"
#include "error.hpp"
#include "flux.hpp"
#include "tinyexpr.h"
#include "util.hpp"
#include <regex>
#include <seanerbus/client/simple_event_loop.hpp>
#include <seanerbus/helpers/resolve.hpp>
#include <seanerbus/msg/message.hpp>
#include <set>
#include <sstream>
#include <stdexcept>
#include <utility>

static const ps_formula_t DEFAULT_JOB_PRIORITY = {
    .name = "job_priority",
    .expr = "<var:prev_priority> * 0.5 + <var:user_score> * 0.5",
    .defaults = {{"var:prev_priority", 16}, {"var:user_score", 16}}};

static const ps_formula_t DEFAULT_JOB_SCORE = {
    .name = "job_score",
    .expr = "<var:duration_ratio> + <var:timewin_score>",
    .defaults = {{"var:timewin_score", 0}, {"var:duration_ratio", 0}}};

static const std::vector<ps_formula_t> DEFAULT_FORMULAS = {DEFAULT_JOB_PRIORITY,
                                                           DEFAULT_JOB_SCORE};

////

json_t *ps_formula_to_json(const ps_formula_t &formula) {
  auto *obj = json_object();
  json_object_set_new(obj, "name", json_string(formula.name.c_str()));
  json_object_set_new(obj, "expr", json_string(formula.expr.c_str()));
  auto *defaults_obj = json_object();
  for (const auto &[var_name, value] : formula.defaults) {
    json_object_set_new(defaults_obj, var_name.c_str(), json_real(value));
  }
  json_object_set_new(obj, "defaults", defaults_obj);
  return obj;
}

ps_formula_t ps_formula_from_json(json_t *obj) {
  ps_formula_t res;
  const char *name = nullptr;
  const char *expr = nullptr;
  json_t *defaults_obj = nullptr;
  json_unpack(obj, "{s:s, s:s, s:o}", "name", &name, "expr", &expr, "defaults",
              &defaults_obj);
  res.name = name ? name : "";
  res.expr = expr ? expr : "";
  if (defaults_obj && json_is_object(defaults_obj)) {
    const char *var_name;
    json_t *value_j;
    json_object_foreach(defaults_obj, var_name, value_j) {
      res.defaults[var_name] = json_real_value(value_j);
    }
  }
  return res;
}

static double min(double a, double b) { return a < b ? a : b; }
static double max(double a, double b) { return a > b ? a : b; }
ps_result_t<double>
ps_eval_formula(const ps_formula_t &formula,
                const std::map<std::string, double> &vars_over) {
  std::map<std::string, double> vars_merged = vars_over;
  for (const auto &[name, value] : formula.defaults) {
    if (vars_merged.find(name) != vars_merged.end())
      continue;
    vars_merged[name] = value;
  }

  std::string transformed_expr = formula.expr;
  std::vector<std::string> var_ids;
  var_ids.reserve(vars_merged.size());

  auto *te_vars =
      (te_variable *)malloc((vars_merged.size() + 2) * sizeof(te_variable));
  int i = 0;

  std::unordered_map<std::string, std::size_t> next_index;
  std::unordered_map<std::string, std::string> assigned_ids;
  for (const auto &[name, value] : vars_merged) {
    // Rewrite "<A:B>" into the identifier "A_vX" so tinyexpr parses a real,
    // prefix-unique variable name; error locations then point at readable text.

    size_t colon = name.find(':');
    const std::string prefix =
        colon == std::string::npos ? name : name.substr(0, colon);

    const std::size_t index = next_index[prefix]++;
    auto var_id = prefix + "_v" + std::to_string(index);
    assigned_ids.emplace(name, var_id);

    var_ids.push_back(var_id);

    std::string pattern = "<" + name + ">";
    size_t pos = 0;
    while ((pos = transformed_expr.find(pattern, pos)) != std::string::npos) {
      transformed_expr.replace(pos, pattern.length(), var_id);
      pos += var_id.length();
    }

    te_vars[i++] = {var_ids.back().c_str(), &value};
  }
  te_vars[i++] = {"min", (void *)min, TE_FUNCTION2};
  te_vars[i++] = {"max", (void *)max, TE_FUNCTION2};

  int err = 0;
  te_expr *compiled = te_compile(transformed_expr.c_str(), te_vars, i, &err);
  if (!compiled) {
    return ps_error(ps_formula_eval_err_t{.formula = formula,
                                          .var_values = vars_merged,
                                          .error_pos = err,
                                          .compiled_expr = transformed_expr});
  }
  auto result = te_eval(compiled);
  te_free(compiled);
  free(te_vars);
  return result;
}

bool ps_validate_formula(const ps_formula_t &formula) {
  auto formula_expr =
      std::regex_replace(formula.expr, std::regex(R"(<[^>]*>)"), "0");
  std::map<std::string, double> vars;
  auto result = ps_eval_formula({.name = formula.name,
                                 .expr = formula_expr,
                                 .defaults = formula.defaults},
                                vars);
  return (ps_is_ok(result));
}

ps_result_t<ps_db_formulas_get_t>
ps_db_formulas_get(const std::vector<std::string> &names) {
  std::vector<std::string> keys;
  for (auto formula_name : names) {
    keys.push_back("priorisea.formula." + formula_name);
  }

  auto lookup_res = ps_flux_kvs_lookup(keys);
  if (!ps_is_ok(lookup_res)) {
    std::stringstream ss;
    ss << "looking up formulas in flux kvs: ";
    for (const auto &key : keys) {
      ss << key << " ";
    }
    return ps_forward_error<ps_db_formulas_get_t>(lookup_res, ss.str());
  }
  std::vector<ps_db_formula_get_entry_t> res;
  for (const auto &lookup_key_res : ps_unwrap(lookup_res)) {
    size_t pos = lookup_key_res.key.rfind('.');
    std::string formula_name = (pos == std::string::npos)
                                   ? lookup_key_res.key
                                   : lookup_key_res.key.substr(pos + 1);

    if (!ps_is_ok(lookup_key_res.value)) {
      res.push_back(
          {.name = formula_name,
           .result = ps_forward_error<ps_formula_t>(
               lookup_key_res.value,
               "looking up formula '" + formula_name + "' in flux kvs")});
      continue;
    }

    auto lkrv = lookup_key_res.value;
    auto lookup_key_unwrap = ps_unwrap(lkrv);

    json_error_t json_err;
    auto *json_formula = json_loads(lookup_key_unwrap.c_str(), 0, &json_err);
    if (!json_formula) {
      res.push_back({.name = formula_name,
                     .result = ps_error(
                         ps_json_unpack_err_t{.dumps = lookup_key_unwrap},
                         "parsing stored formula '" + formula_name + "'")});
      continue;
    }

    auto formula = ps_formula_from_json(json_formula);
    json_decref(json_formula);
    res.push_back({.name = formula_name, .result = formula});
  }

  return res;
};

ps_result_t<ps_formula_t> ps_db_formula_get(const std::string &name) {
  auto get_res = ps_db_formulas_get({name});
  if (!ps_is_ok(get_res)) {
    return ps_forward_error<ps_formula_t>(get_res, "getting formula '" + name +
                                                       "' from flux kvs");
  }
  auto get_unwrap = ps_unwrap(get_res);
  return get_unwrap[0].result;
};

ps_result_t<ps_success_t> ps_db_formula_set(const ps_formula_t &formula) {
  if (!ps_validate_formula(formula)) {
    return ps_error(ps_formula_expr_invalid_t{.expr = formula.expr});
  }

  if (formula.name == "job_priority") {
    auto plugin_set_res = ps_flux_plugin_set_formula(formula);
    if (!ps_is_ok(plugin_set_res)) {
      return ps_add_context(plugin_set_res, "sending formula to flux plugin");
    }
  }

  // TODO: if the store is rejected, roll
  // back the priority formula in the flux plugin
  auto kvs_key = "priorisea.formula." + formula.name;
  auto *json_formula = ps_formula_to_json(formula);
  char *formula_str = json_dumps(json_formula, 0);
  json_decref(json_formula);
  if (!formula_str) {
    return ps_error(ps_json_pack_err_t{});
  }

  auto txn = flux_kvs_txn_create();
  if (!txn) {
    free(formula_str);
    return ps_error(ps_flux_kvs_txn_create_err_t{.msg = strerror(errno)});
  }
  if (flux_kvs_txn_put(txn, 0, kvs_key.c_str(), formula_str) < 0) {
    free(formula_str);
    return ps_error(ps_flux_kvs_txn_put_err_t{
        .key = kvs_key, .value = formula_str, .msg = strerror(errno)});
  }
  free(formula_str);
  auto commit_res = ps_flux_kvs_commit(txn);
  if (!ps_is_ok(commit_res)) {
    return ps_forward_error<ps_success_t>(
        commit_res, "committing formula '" + formula.name + "' to flux kvs");
  }
  return ps_success_t{};
}

ps_result_t<ps_success_t>
ps_db_formulas_reset(const std::vector<std::string> &names) {
  for (const auto &name : names) {
    bool found = false;
    ps_formula_t formula;

    for (const auto &default_formula : DEFAULT_FORMULAS) {
      if (name == default_formula.name) {
        formula = default_formula;
        found = true;
        break;
      }
    }

    if (!found) {
      return ps_error(ps_formula_invalid_name_t{.name = name});
    }

    auto update_res = ps_db_formula_set(formula);
    if (!ps_is_ok(update_res)) {
      return ps_forward_error<ps_success_t>(
          update_res, "setting formula '" + name + "' in database");
    }
  }
  return ps_success_t{};
}

ps_result_t<ps_success_t> ps_db_formulas_init() {
  std::vector<std::string> default_formulas_name;
  for (const auto &default_formula : DEFAULT_FORMULAS) {
    default_formulas_name.push_back(default_formula.name);
  }

  auto get_res = ps_db_formulas_get(default_formulas_name);
  if (!ps_is_ok(get_res)) {
    return ps_forward_error<ps_success_t>(
        get_res, "getting default formulas from database");
  }

  auto get_unwrap = ps_unwrap(get_res);
  std::vector<std::string> missing_formulas;
  for (const auto &get_formula_res : get_unwrap) {
    if (!(ps_is_ok(get_formula_res.result))) {
      missing_formulas.push_back(get_formula_res.name);
    }
  }

  auto reset_res = ps_db_formulas_reset(missing_formulas);
  if (!ps_is_ok(reset_res)) {
    std::stringstream ss;
    ss << "resetting missing formulas in database: ";
    for (const auto &formula : missing_formulas) {
      ss << formula << " ";
    }

    return ps_forward_error<ps_success_t>(reset_res, ss.str());
  }

  return ps_success_t{};
}

ps_result_t<std::vector<ps_db_formula_get_entry_t>> ps_db_formulas_list() {
  auto ls_res = ps_flux_kvs_ls_dir("priorisea.formula");
  if (!ps_is_ok(ls_res)) {
    return ps_forward_error<ps_db_formulas_get_t>(
        ls_res, "listing formulas in flux kvs");
  }
  return ps_db_formulas_get(ps_unwrap(ls_res));
}

ps_result_t<std::map<std::string, ps_result_t<double>>>
ps_resolve_seanerbus_vars(const ps_formula_t &formula,
                          const flux_jobid_t &jobid,
                          const ps_seanerbus_t &seanerbus) {
  std::map<std::string, ps_result_t<double>> sb_results;

  std::regex rpc_pattern(R"(<rpc:[^>]*>)");
  std::regex rpc_ns_pattern(R"(<rpc_ns:([^>]*)>)");
  std::regex adapter_pattern(R"(<adapter:[^>]*>)");

  auto collect = [&formula](const std::regex &pattern) {
    std::set<std::string> matches;
    for (auto it = std::sregex_iterator(formula.expr.begin(),
                                        formula.expr.end(), pattern);
         it != std::sregex_iterator(); ++it) {
      matches.insert((*it)[0].str());
    }
    return matches;
  };

  std::set<std::string> rpc_matches = collect(rpc_pattern);
  std::set<std::string> rpc_ns_matches = collect(rpc_ns_pattern);
  std::set<std::string> adapter_matches = collect(adapter_pattern);

  if (rpc_matches.empty() && rpc_ns_matches.empty() &&
      adapter_matches.empty()) {
    return sb_results;
  }

  const std::string endpoint =
      "demobus://" + seanerbus.ip + ":" + std::to_string(seanerbus.port);

  // Send a HelloWorldReqV1 carrying the job id to the given entity and store
  // the parsed double reply (or an error) under service_full.
  auto do_request = [&](const std::string &service_full,
                        const seanerbus::msg::Entity &entity) {
    if (sb_results.find(service_full) != sb_results.end()) {
      return;
    }
    try {
      seanerbus::client::Connection req_conn(endpoint, endpoint);
      seanerbus::msg::MessageBuilder<HelloWorldReqV1> bd;
      bd.builder().setMsg(std::to_string(jobid));
      req_conn.request(
          bd.to_message(entity),
          [&](seanerbus::client::Connection &c,
              seanerbus::Expected<seanerbus::msg::Message, seanerbus::Error>
                  res) {
            if (!res.ok()) {
              sb_results[service_full] = ps_error(ps_seanerbus_err_t{
                  .service = service_full, .msg = res.unwrap_error().what()});
              c.shutdown(seanerbus::Unexpected(res.unwrap_error()));
              return;
            }
            auto msg = std::move(res.unwrap_ok());
            if (msg.payload_type() != HelloWorldResV1::TYPE) {
              sb_results[service_full] = ps_error(
                  ps_seanerbus_wrong_payload_err_t{.service = service_full});
              c.shutdown();
              return;
            }
            auto rd_res =
                seanerbus::msg::MessageReader<HelloWorldResV1>::from_msg(msg);
            if (!rd_res.ok()) {
              sb_results[service_full] = ps_error(ps_seanerbus_decode_err_t{
                  .service = service_full,
                  .msg = rd_res.unwrap_error().what()});
              c.shutdown();
              return;
            }
            auto rd = std::move(rd_res.unwrap_ok());
            const std::string res_str = rd.reader().getMsg().cStr();
            double res_double;
            if (!parse_double(res_str, res_double)) {
              sb_results[service_full] =
                  ps_error(ps_parse_double_err_t{.double_str = res_str},
                           "seanerbus rpc (" + service_full + ")");
              c.shutdown();
              return;
            }
            sb_results[service_full] = res_double;
            c.shutdown();
          });
      seanerbus::client::SimpleEventLoop req_sel(req_conn);
      req_sel.run();
    } catch (const std::exception &e) {
      sb_results[service_full] = ps_error(ps_seanerbus_connection_err_t{
          .seanerbus = seanerbus, .msg = e.what()});
    }
  };

  // <rpc:UUID>: the inner text is the entity UUID, used directly.
  for (const std::string &match : rpc_matches) {
    const std::string rpc_service_full =
        match.substr(1, match.length() - 2); // rpc:UUID
    const std::string rpc_service =
        rpc_service_full.substr(4, rpc_service_full.length() - 4); // UUID

    seanerbus::msg::Entity entity;
    try {
      entity = seanerbus::msg::Entity(seanerbus::msg::Uuid(rpc_service));
    } catch (const std::invalid_argument &) {
      sb_results[rpc_service_full] =
          ps_error(ps_invalid_uuid_err_t{.uuid = rpc_service});
      continue;
    }
    do_request(rpc_service_full, entity);
  }

  // <rpc_ns:{...}>: the inner text is a JSON object (a map<string,string>)
  // naming the entity to resolve via the naming service, e.g.
  // <rpc_ns:{"service":"time_series_db"}>.
  for (const std::string &match : rpc_ns_matches) {
    // Key by the token content (name), matching what ps_eval_formula rebuilds
    // as "<" + name + ">" and what the rpc path already does.
    const std::string rpc_ns_service_full =
        match.substr(1, match.length() - 2); // rpc_ns:{...}

    std::smatch json_m;
    if (!std::regex_search(match, json_m, rpc_ns_pattern) ||
        json_m.size() < 2) {
      sb_results[rpc_ns_service_full] =
          ps_error(ps_invalid_rpc_ns_err_t{.expr = match});
      continue;
    }
    const std::string rpc_ns_json = json_m[1].str();

    json_error_t json_err;
    json_t *json_obj = json_loads(rpc_ns_json.c_str(), 0, &json_err);
    if (!json_obj || !json_is_object(json_obj)) {
      sb_results[rpc_ns_service_full] =
          ps_error(ps_invalid_rpc_ns_err_t{.expr = rpc_ns_json});
      json_decref(json_obj);
      continue;
    }

    std::map<std::string, std::string> resolve_name;
    const char *key = nullptr;
    json_t *value_j = nullptr;
    json_object_foreach(json_obj, key, value_j) {
      if (json_is_string(value_j)) {
        resolve_name[key] = json_string_value(value_j);
      }
    }
    json_decref(json_obj);

    seanerbus::msg::Entity entity;
    bool resolved = false;
    {
      seanerbus::client::Connection conn(endpoint, endpoint);
      seanerbus::resolve::resolve(
          conn, resolve_name,
          [&](seanerbus::client::Connection &c,
              seanerbus::Expected<seanerbus::resolve::ResolveResult,
                                  seanerbus::Error>
                  res) {
            if (!res.ok()) {
              c.shutdown(seanerbus::Unexpected(res.unwrap_error()));
              return;
            }
            auto result = res.unwrap_ok();
            auto entities = result.entities();
            if (!entities.empty()) {
              entity = entities[0];
              resolved = true;
            }
            c.shutdown();
          });
      seanerbus::client::SimpleEventLoop sel(conn);
      sel.run();
    }

    if (!resolved) {
      sb_results[rpc_ns_service_full] =
          ps_error(ps_seanerbus_resolve_err_t{.service = rpc_ns_json});
      continue;
    }
    do_request(rpc_ns_service_full, entity);
  }

  for (const std::string &match : adapter_matches) {
    std::string inner = match.substr(2, match.length() - 4);
    if (sb_results.find(inner) != sb_results.end()) {
      continue;
    }
    sb_results[inner] =
        ps_error(ps_unimplemented_err_t{}, "seanerbus adapter (" + inner + ")");
  }
  return sb_results;
}
