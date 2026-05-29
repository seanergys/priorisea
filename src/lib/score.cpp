// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "score.hpp"
#include "decay.hpp"
#include "flux.hpp"
#include "tinyexpr.h"
#include "util.hpp"
#include <chrono>

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

eval_formula_job_score_res eval_formula_job_score(formula formula_obj,
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

bool validate_formula_job_score(formula formula_obj) {
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

get_users_res get_users(std::vector<int> &userids) {
  get_users_res res;
  res.ok = false;

  std::vector<std::string> keys;
  for (const auto &userid : userids) {
    keys.push_back("priorisea.user." + std::to_string(userid));
  }
  auto kvs_res = kvs_lookup(keys);
  if (kvs_res.err != FLUX_KVS_ERR_SUCCESS) {
    res.message = "Failed to lookup KVS. Unknown error";
    return res;
  }

  for (const auto &lookup_key_res : kvs_res.res) {
    get_user_res user_res;
    user_res.ok = false;

    size_t pos = lookup_key_res.key.rfind('.');
    std::string userid_str = (pos == std::string::npos)
                                 ? lookup_key_res.key
                                 : lookup_key_res.key.substr(pos + 1);

    int userid;
    if (!parse_int(userid_str, userid)) {
      user_res.message = "Failed to parse userid from KVS key: " + userid_str;
      res.res.push_back(user_res);
      continue;
    }
    user_res.userid = userid;

    if (lookup_key_res.err != FLUX_KVS_ERR_SUCCESS) {
      user_res.message = "Failed to lookup KVS. Unknown error: " +
                         std::to_string(lookup_key_res.err);
      res.res.push_back(user_res);
      continue;
    }

    double score;
    if (!parse_double(lookup_key_res.value, score)) {
      user_res.message =
          "Failed to parse score from KVS value: " + lookup_key_res.value;
      res.res.push_back(user_res);
      continue;
    }

    user_res.ok = true;
    user_res.user_obj = {.userid = userid, .score = score};
    res.res.push_back(user_res);
  }

  res.ok = true;
  return res;
};

store_user_res store_user(user user_obj) {
  store_user_res res;
  res.ok = false;
  auto txn = flux_kvs_txn_create();
  if (!txn) {
    res.message = "failed to create KVS transaction";
    return res;
  }
  if (flux_kvs_txn_put(
          txn, 0, ("priorisea.user." + std::to_string(user_obj.userid)).c_str(),
          std::to_string(user_obj.score).c_str()) < 0) {
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

update_job_res update_job(flux_list_job_res job_obj, formula formula_obj) {
  update_job_res res;
  res.ok = false;

  std::map<std::string, double> vars_over;
  auto eval_res = eval_formula_job_score(formula_obj, vars_over);
  if (!eval_res.ok) {
    res.message = "failed to evaluate formula";
    return res;
  }

  auto store_job_res = store_job({std::to_string(job_obj.id), eval_res.result});
  if (!store_job_res.ok) {
    res.message = "failed to store job score in KVS";
    return res;
  }

  res.ok = true;
  return res;
}

update_users_res update_users() {
  // TODO: if a user has not submitted any job in the entire decay period,
  // the score will not be updated. That user will thus keep its current score.
  // until the next time they run a job
  update_users_res res;
  res.ok = false;

  auto get_decay_res = get_decay();
  if (!get_decay_res.ok) {
    res.message = get_decay_res.message;
    return res;
  }
  auto decay_obj = get_decay_res.decay_obj;

  // timestamp is the latest time possible where decay applies
  auto now = std::chrono::system_clock::now();
  auto timestamp =
      std::chrono::duration<double>(now.time_since_epoch()).count();
  for (const auto &decay_one_obj : get_decay_res.decay_obj) {
    timestamp -= decay_one_obj.duration;
  }

  // structure to store sum and count of scores for each user for each time slot
  // of decay
  struct sum_agg {
    double sum;
    size_t count;
  };
  std::map<int, std::vector<sum_agg>> sum_aggs;

  flux_list_inactive_stream(
      timestamp, 10, [&sum_aggs, &decay_obj, &timestamp](json_t *jobs) {
        std::vector<std::string> jobids;
        struct job_info {
          int decay_index;
          flux_list_job_res list_job_res;
        };
        std::map<std::string, job_info> jobid_info;

        size_t index;
        json_t *value;
        json_array_foreach(jobs, index, value) {
          auto unpack_res = flux_list_job_unpack(value);
          if (!unpack_res.ok) {
            std::cerr << "Error: failed to unpack job: " << unpack_res.message
                      << '\n';
            continue;
          }

          // find the index of the decay step that applies to this job
          auto _timestamp = timestamp;
          int i = decay_obj.size() - 1;
          for (; i >= 0; i--) {
            _timestamp += decay_obj[i].duration;
            if (_timestamp > unpack_res.job.t_inactive) {
              break;
            }
          }

          jobid_info[std::to_string(unpack_res.job.id)] = {
              .decay_index = i, .list_job_res = unpack_res.job};
        }

        for (const auto &[key, _] : jobid_info) {
          jobids.push_back(key);
        }
        auto get_jobs_res = get_jobs(jobids);
        if (!get_jobs_res.ok) {
          std::cerr << "Error: failed to get jobs: " << get_jobs_res.message
                    << '\n';
          return;
        }

        for (const auto &get_job_res : get_jobs_res.res) {
          if (!get_job_res.ok) {
            std::cerr << "Error: failed to get job: " << get_job_res.message
                      << '\n';
            continue;
          }
          auto job_inf = jobid_info[get_job_res.jobid];
          auto userid = job_inf.list_job_res.userid;

          if (sum_aggs.find(userid) == sum_aggs.end()) {
            sum_aggs[userid] = std::vector<sum_agg>(decay_obj.size());
            for (size_t i = 0; i < decay_obj.size(); i++) {
              sum_aggs[userid][i] = {0, 0};
            }
          }

          auto *curr_agg = &sum_aggs[userid][job_inf.decay_index];
          curr_agg->sum += get_job_res.job_obj.score *
                           decay_obj[job_inf.decay_index].priority;
          curr_agg->count++;
        }
      });

  std::map<int, double> scores;
  for (const auto &[userid, value] : sum_aggs) {
    double score = 0;
    for (const auto &agg : value) {
      if (agg.count > 0) {
        score += agg.sum / agg.count;
      }
    }
    scores[userid] = score;
    auto store_res = store_user({.userid = userid, .score = score});
    if (!store_res.ok) {
      std::cerr << "Error: failed to store user score: " << store_res.message
                << '\n';
    }
  }

  auto plugin_set_scores_res = flux_plugin_set_score(scores);
  if (plugin_set_scores_res.err != FLUX_PLUGIN_ERR_OK) {
    res.message = plugin_set_scores_res.err_msg;
    return res;
  }

  res.ok = true;
  return res;
}

get_users_res list_users() {
  get_users_res res;
  res.ok = false;

  auto ls_res = flux_kvs_ls_dir("priorisea.user");
  if (!ls_res.ok) {
    res.message = ls_res.message;
    return res;
  }

  std::vector<int> userids;
  for (const auto &userid_str : ls_res.keys) {
    int userid;
    if (!parse_int(userid_str, userid)) {
      std::cerr << "Error: failed to parse userid: " << userid_str << '\n';
      continue;
    }
    userids.push_back(userid);
  }

  return get_users(userids);
}

get_jobs_res list_jobs() {
  get_jobs_res res;
  res.ok = false;

  auto ls_res = flux_kvs_ls_dir("priorisea.job");
  if (!ls_res.ok) {
    res.message = ls_res.message;
    return res;
  }
  return get_jobs(ls_res.keys);
}
