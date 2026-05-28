// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "score.hpp"
#include "flux.hpp"
#include "tinyexpr.h"
#include "util.hpp"

static double min(double a, double b) { return a < b ? a : b; }
static double max(double a, double b) { return a > b ? a : b; }

////

get_jobs_res get_jobs(std::vector<std::string> &jobids) {
  get_jobs_res res;
  res.ok = false;

  std::vector<std::string> keys;
  for (const auto &jobid : jobids) {
    keys.push_back("priorisea.job." + jobid);
  }
  auto kvs_res = kvs_lookup(keys);
  if (kvs_res.err != FLUX_KVS_ERR_SUCCESS) {
    res.message = "Failed to lookup KVS. Unknown error";
    return res;
  }

  for (const auto &lookup_key_res : kvs_res.res) {
    get_job_res job_res;
    job_res.ok = false;

    size_t pos = lookup_key_res.key.rfind('.');
    std::string jobid = (pos == std::string::npos)
                            ? lookup_key_res.key
                            : lookup_key_res.key.substr(pos + 1);
    job_res.jobid = jobid;

    if (lookup_key_res.err != FLUX_KVS_ERR_SUCCESS) {
      job_res.message = "Failed to lookup KVS. Unknown error: " +
                        std::to_string(lookup_key_res.err);
      res.res.push_back(job_res);
      continue;
    }

    double score;
    if (!parse_double(lookup_key_res.value, score)) {
      job_res.message =
          "Failed to parse score from KVS value: " + lookup_key_res.value;
      res.res.push_back(job_res);
      continue;
    }

    job_res.ok = true;
    job_res.job_obj = {.jobid = jobid, .score = score};
    res.res.push_back(job_res);
  }

  res.ok = true;
  return res;
};

eval_formula_res eval_formula(formula formula_obj,
                              std::map<std::string, double> &vars_over) {

  std::map<std::string, double> vars_merged = vars_over;
  for (const auto &[name, value] : formula_obj.vars) {
    if (vars_merged.find(name) != vars_merged.end())
      continue;
    vars_merged[name] = value;
  }

  auto *te_vars =
      (te_variable *)malloc((vars_merged.size() + 2) * sizeof(te_variable));
  int i = 0;

  for (const auto &[name, value] : vars_merged) {
    te_vars[i++] = {name.c_str(), &value};
  }
  te_vars[i++] = {"min", (void *)min, TE_FUNCTION2};
  te_vars[i++] = {"max", (void *)max, TE_FUNCTION2};

  int err = 0;
  te_expr *compiled = te_compile(formula_obj.expr.c_str(), te_vars, i, &err);
  if (!compiled)
    return {false, 0};
  auto result = te_eval(compiled);
  te_free(compiled);
  return {true, result};
}

bool validate_score(formula formula_obj) {
  auto *te_vars =
      (te_variable *)malloc(formula_obj.vars.size() * sizeof(te_variable));
  int i = 0;
  for (const auto &[name, value] : formula_obj.vars) {
    te_vars[i++] = {name.c_str(), &value};
  }
  int err = 0;
  te_expr *compiled = te_compile(formula_obj.expr.c_str(), te_vars, i, &err);
  if (!compiled)
    return false;
  te_free(compiled);
  return true;
}

store_t_inactive_res store_t_inactive(double t_inactive) {
  store_t_inactive_res res;
  res.ok = false;
  auto txn = flux_kvs_txn_create();
  if (!txn) {
    res.message = "failed to create KVS transaction";
    return res;
  }
  if (flux_kvs_txn_put(txn, 0, "priorisea.t_inactive",
                       std::to_string(t_inactive).c_str()) < 0) {
    res.message = "failed to store max_t_inactive in KVS";
    return res;
  }
  auto commit_res = kvs_commit(txn);
  if (commit_res != FLUX_KVS_ERR_SUCCESS) {
    res.message = "failed to commit max_t_inactive to KVS";
    return res;
  }
  res.ok = true;
  return res;
}

get_t_inactive_res get_t_inactive() {
  get_t_inactive_res res;
  res.ok = false;
  auto lookup_res = kvs_lookup({"priorisea.t_inactive"});
  if (lookup_res.err != FLUX_KVS_ERR_SUCCESS) {
    res.message = "failed to lookup t_inactive in KVS";
    return res;
  }
  if (lookup_res.res[0].err != FLUX_KVS_ERR_SUCCESS) {
    res.message = "failed to lookup t_inactive in KVS";
    return res;
  }
  double t_inactive;
  if (!parse_double(lookup_res.res[0].value, res.t_inactive)) {
    res.message = "failed to parse t_inactive value";
    return res;
  }
  res.ok = true;
  return res;
}

store_job_res store_job(job job_obj) {
  store_job_res res;
  res.ok = false;
  auto txn = flux_kvs_txn_create();
  if (!txn) {
    res.message = "failed to create KVS transaction";
    return res;
  }
  if (flux_kvs_txn_put(txn, 0, ("priorisea.job." + job_obj.jobid).c_str(),
                       std::to_string(job_obj.score).c_str()) < 0) {
    res.message = "failed to store job score in KVS";
    return res;
  }
  auto commit_res = kvs_commit(txn);
  if (commit_res != FLUX_KVS_ERR_SUCCESS) {
    res.message = "failed to commit job score to KVS";
    return res;
  }
  res.ok = true;
  return res;
}

init_score_res init_score() {
  init_score_res res;
  res.ok = false;
  auto get_t_inactive_res = get_t_inactive();
  if (!get_t_inactive_res.ok) {
    auto store_t_inactive_res = store_t_inactive(0);
    if (!store_t_inactive_res.ok) {
      res.message = store_t_inactive_res.message;
      return res;
    }
  }
  res.ok = true;
  return res;
}
