#include "src/lib/formula.hpp"
#include "external/tinyexpr/tinyexpr.h"
#include "src/lib/error.hpp"
#include <cstdlib>
#include <math.h>
#include <regex>
#include <unordered_map>
#include <utility>

json_t *ps_formula_to_json(const ps_formula_t &formula) {
  auto *obj = json_object();
  json_object_set_new(obj, "name", json_string(formula.name.c_str()));
  json_object_set_new(obj, "expr", json_string(formula.expr.c_str()));
  json_object_set_new(obj, "defaults",
                      ps_formula_vars_to_json(formula.defaults));
  return obj;
}

ps_result_t<ps_formula_t> ps_formula_from_json(json_t *obj) {
  ps_formula_t res;
  json_t *name_j = json_object_get(obj, "name");
  if (!name_j) {
    return ps_error(ps_field_missing_t{.field = "name"});
  }
  if (!json_is_string(name_j)) {
    return ps_error(
        ps_field_type_mismatch_t{.field = "name", .expected = "string"});
  }
  res.name = json_string_value(name_j);
  json_t *expr_j = json_object_get(obj, "expr");
  if (!expr_j) {
    return ps_error(ps_field_missing_t{.field = "expr"});
  }
  if (!json_is_string(expr_j)) {
    return ps_error(
        ps_field_type_mismatch_t{.field = "expr", .expected = "string"});
  }
  res.expr = json_string_value(expr_j);
  json_t *defaults_obj = json_object_get(obj, "defaults");
  if (!defaults_obj) {
    return res;
  }
  if (!json_is_object(defaults_obj)) {
    return ps_error(
        ps_field_type_mismatch_t{.field = "defaults", .expected = "object"});
  }
  auto defaults_res = ps_formula_vars_from_json(defaults_obj);
  if (!ps_is_ok(defaults_res)) {
    return ps_forward_error<ps_formula_t>(defaults_res);
  }
  res.defaults = ps_unwrap(defaults_res);
  return res;
}

json_t *ps_formula_vars_to_json(const std::map<std::string, double> &vars) {
  auto *obj = json_object();
  for (const auto &[name, value] : vars) {
    json_object_set_new(obj, name.c_str(), json_real(value));
  }
  return obj;
}

ps_result_t<std::map<std::string, double>>
ps_formula_vars_from_json(json_t *obj) {
  std::map<std::string, double> res;
  if (!json_is_object(obj)) {
    return ps_error(
        ps_field_type_mismatch_t{.field = "variables", .expected = "object"});
  }
  const char *name;
  json_t *value_j;
  json_object_foreach(obj, name, value_j) {
    if (!json_is_real(value_j)) {
      auto key_str = std::string(name);
      return ps_error(
          ps_field_type_mismatch_t{.field = key_str, .expected = "double"});
    }
    res[name] = json_real_value(value_j);
  }
  return res;
}

std::string
ps_formula_vars_to_json_str(const std::map<std::string, double> &vars) {
  auto *obj = ps_formula_vars_to_json(vars);
  char *text = json_dumps(obj, JSON_COMPACT);
  json_decref(obj);
  std::string result(text);
  free(text);
  return result;
}

ps_result_t<std::map<std::string, double>>
ps_formula_vars_from_json_str(const std::string &text) {
  std::map<std::string, double> res;
  if (text.empty()) {
    return res;
  }
  json_t *obj = json_loads(text.c_str(), 0, nullptr);
  if (!obj) {
    return ps_error(ps_unexpected_err_t{.msg = "invalid variables JSON"});
  }
  auto parsed = ps_formula_vars_from_json(obj);
  json_decref(obj);
  return parsed;
}

static double _min(double a, double b) { return a < b ? a : b; }
static double _max(double a, double b) { return a > b ? a : b; }
static double _pow(double a, double b) { return pow(a, b); }
static double _exp(double v) { return exp(v); }

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

  // Number of variables + number of functions
  auto *te_vars =
      (te_variable *)malloc((vars_merged.size() + 4) * sizeof(te_variable));
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
  te_vars[i++] = {"exp", (void *)_exp, TE_FUNCTION1};
  te_vars[i++] = {"min", (void *)_min, TE_FUNCTION2};
  te_vars[i++] = {"max", (void *)_max, TE_FUNCTION2};
  te_vars[i++] = {"pow", (void *)_pow, TE_FUNCTION2};

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

ps_result_t<ps_success_t> ps_validate_formula(const ps_formula_t &formula) {
  auto formula_expr =
      std::regex_replace(formula.expr, std::regex(R"(<[^>]*>)"), "0");
  std::map<std::string, double> vars;
  auto result = ps_eval_formula({.name = formula.name,
                                 .expr = formula_expr,
                                 .defaults = formula.defaults},
                                vars);
  if (!ps_is_ok(result)) {
    return ps_forward_error<ps_success_t>(result);
  }
  return ps_success_t{};
}
