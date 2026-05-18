// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <jansson.h>
#include <map>
#include <string>
#include <vector>

enum FLUX_PLUGIN_ERR {
  FLUX_PLUGIN_ERR_OK,
  FLUX_PLUGIN_ERR_OPEN,
  FLUX_PLUGIN_ERR_NOT_LOADED,
  FLUX_PLUGIN_ERR_BAD_RESPONSE,
};

struct flux_plugin_res {
  FLUX_PLUGIN_ERR err;
  std::string err_msg;
};

struct flux_plugin_get_formula_res : flux_plugin_res {
  std::string formula;
};
flux_plugin_get_formula_res flux_plugin_get_formula();

struct flux_plugin_set_formula_res : flux_plugin_res {
  bool formula_set;
};
flux_plugin_set_formula_res flux_plugin_set_formula(std::string formula);

struct flux_plugin_get_score_res : flux_plugin_res {
  std::map<int, int> scores;
};
flux_plugin_get_score_res flux_plugin_get_score(std::vector<int> user_ids);

struct flux_plugin_set_score_res : flux_plugin_res {
  bool score_set;
};
flux_plugin_set_score_res flux_plugin_set_score(std::map<int, int> scores);
