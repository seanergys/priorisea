#pragma once
#include "src/lib/types.hpp"

/**
 * Plugin debug data with the formula and per-user scores
 */
struct ps_cli_endpoint_plugin_debug_t {
  ps_formula_t formula;
  std::map<uid_t, double> scores;
};
