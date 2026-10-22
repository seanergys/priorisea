#include "src/lib/error.hpp"
#include <sstream>
#include <string>

std::string ps_error_message(const ps_unexpected_err_t &err) {
  return "Unexpected error: " + err.msg;
}

std::string ps_error_message(const ps_permissions_err_t &err) {
  return "Permission error: " + err.msg;
}

std::string ps_error_message(const ps_access_denied_t &err) {
  return "Access denied to " + err.key;
}

std::string ps_error_message(const ps_field_missing_t &err) {
  return "Field " + err.field + " is missing";
}

std::string ps_error_message(const ps_field_type_mismatch_t &err) {
  return "Field " + err.field + " expected the following type: " + err.expected;
}

std::string ps_error_message(const ps_string_parse_err_t &err) {
  return "Error parsing string: " + err.msg;
}

std::string ps_error_message(const ps_formula_eval_err_t &err) {
  std::string msg = "formula eval error. name: " + err.formula.name + "\n" +
                    err.compiled_expr;

  if (err.error_pos > 0) {
    size_t caret = static_cast<size_t>(err.error_pos) - 1;
    if (caret > err.compiled_expr.length())
      caret = err.compiled_expr.length();
    msg += "\n" + std::string(caret, ' ') + "^";
  }

  if (!err.formula.expr.empty() && err.formula.expr != err.compiled_expr)
    msg += "\n(original: " + err.formula.expr + ")";

  std::string var_err;
  for (const auto &[var_name, value] : err.var_values) {
    if (!var_err.empty())
      var_err += ", ";
    var_err += var_name + ": " + std::to_string(value);
  }
  if (!var_err.empty())
    msg += "\nvariables: " + var_err;

  return msg;
}

std::string ps_error_message(const ps_flux_open_err_t &err) {
  return "Error reaching Flux: " + err.strerror + ". Is Flux running ?";
}

std::string ps_error_message(const ps_flux_rpc_err_t &err) {
  return "Error on Flux RPC: " + err.strerror;
}

std::string ps_error_message(const ps_flux_jobid_parse_err_t &err) {
  return "Error parsing job id " + err.jobid + ": " + err.strerror;
}

ps_error_t ps_error(ps_error_kind_t kind, std::string ctx) {
  ps_error_t e{.kind = kind};
  if (!ctx.empty()) {
    e.context.push_back(std::move(ctx));
  }
  return e;
}

std::string ps_error_message(const ps_error_kind_t &error) {
  return std::visit([](const auto &err) { return ps_error_message(err); },
                    error);
}

std::string ps_error_message(const ps_error_t &e) {
  std::stringstream ss;
  for (const auto &context : e.context) {
    ss << context << "\n-> ";
  }

  ss << std::visit([](const auto &err) { return ps_error_message(err); },
                   e.kind);
  return ss.str();
}

void ps_exit_error(const ps_error_t &e) {
  std::cerr << ps_error_message(e) << std::endl;
  std::exit(EXIT_FAILURE);
}
