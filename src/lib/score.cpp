// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "score.hpp"
#include "croncpp.h"
#include "decay.hpp"
#include "error.hpp"
#include "flux.hpp"
#include "formula.hpp"
#include "timewin.hpp"
#include "types.hpp"
#include "util.hpp"
#include <algorithm>
#include <chrono>
#include <climits>
#include <cmath>
#include <flux/core/job.h>
#include <iostream>
#include <sstream>
#include <sys/types.h>

ps_result_t<ps_db_jobs_get_t>
ps_db_jobs_get(const std::vector<flux_jobid_t> &jobids) {
  ps_db_jobs_get_t res;
  std::vector<std::string> keys;
  for (const auto &jobid : jobids) {
    keys.push_back("priorisea.job." + std::to_string(jobid));
  }
  auto kvs_res = ps_flux_kvs_lookup(keys);
  if (!ps_is_ok(kvs_res)) {
    std::stringstream ss;
    ss << "looking up jobs in flux kvs: ";
    for (const auto &key : keys) {
      ss << key << " ";
    }
    return ps_forward_error<ps_db_jobs_get_t>(kvs_res, ss.str());
  }

  for (const auto &lookup_key_res : ps_unwrap(kvs_res)) {
    size_t pos = lookup_key_res.key.rfind('.');
    auto jobid_str = (pos == std::string::npos)
                         ? lookup_key_res.key
                         : lookup_key_res.key.substr(pos + 1);
    flux_jobid_t jobid;
    if (flux_job_id_parse(jobid_str.c_str(), &jobid) < 0) {
      res.push_back({.jobid = 0,
                     .result = ps_error(ps_flux_jobid_parse_err_t{
                         .jobid = jobid_str, .msg = strerror(errno)})});
      continue;
    }

    if (!ps_is_ok(lookup_key_res.value)) {
      res.push_back({.jobid = jobid,
                     .result = ps_forward_error<ps_job_t>(
                         lookup_key_res.value,
                         "looking up job " + jobid_str + " in flux kvs")});
      continue;
    }
    auto lkrv = lookup_key_res.value;
    auto lookup_key_res_value = ps_unwrap(lkrv);
    double score;
    if (!parse_double(lookup_key_res_value, score)) {
      res.push_back({.jobid = jobid,
                     .result = ps_error(ps_parse_double_err_t{
                         .double_str = lookup_key_res_value})});
      continue;
    }
    res.push_back(
        {.jobid = jobid, .result = ps_job_t{.jobid = jobid, .score = score}});
  }
  return res;
};

ps_result_t<ps_success_t> ps_db_t_inactive_set(double t_inactive) {
  auto txn = flux_kvs_txn_create();
  if (!txn) {
    return ps_error(ps_flux_kvs_txn_create_err_t{.msg = strerror(errno)});
  }
  auto key = "priorisea.t_inactive";
  auto value = std::to_string(t_inactive);
  if (flux_kvs_txn_put(txn, 0, key, value.c_str()) < 0) {
    return ps_error(ps_flux_kvs_txn_put_err_t{
        .key = key, .value = value, .msg = strerror(errno)});
  }
  return ps_flux_kvs_commit(txn);
}

ps_result_t<double> ps_db_t_inactive_get() {
  auto lookup_res = ps_flux_kvs_lookup("priorisea.t_inactive");
  if (!ps_is_ok(lookup_res)) {
    return ps_forward_error<double>(lookup_res,
                                    "looking up t_inactive in flux kvs");
  }
  auto lookup_value = ps_unwrap(lookup_res);
  double t_inactive;
  if (!parse_double(lookup_value, t_inactive)) {
    return ps_error(ps_parse_double_err_t{.double_str = lookup_value});
  }
  return t_inactive;
}

ps_result_t<ps_success_t> ps_db_job_set(const ps_job_t &job) {
  auto txn = flux_kvs_txn_create();
  if (!txn) {
    return ps_error(ps_flux_kvs_txn_create_err_t{.msg = strerror(errno)});
  }
  auto key = "priorisea.job." + std::to_string(job.jobid);
  auto value = std::to_string(job.score);
  if (flux_kvs_txn_put(txn, 0, key.c_str(), value.c_str()) < 0) {
    return ps_error(ps_flux_kvs_txn_put_err_t{
        .key = key, .value = value, .msg = strerror(errno)});
  }
  return ps_flux_kvs_commit(txn);
}

ps_result_t<ps_success_t> ps_db_score_init() {
  auto get_t_inactive_res = ps_db_t_inactive_get();
  if (!ps_is_ok(get_t_inactive_res)) {
    return ps_db_t_inactive_set(0);
  }
  return ps_success_t{};
}

ps_result_t<ps_db_users_get_t>
ps_db_users_get(const std::vector<uid_t> &userids) {
  std::vector<std::string> keys;
  for (const auto &userid : userids) {
    keys.push_back("priorisea.user." + std::to_string(userid));
  }
  auto kvs_res = ps_flux_kvs_lookup(keys);
  if (!ps_is_ok(kvs_res)) {
    std::stringstream ss;
    ss << "looking up users in flux kvs: ";
    for (const auto &key : keys) {
      ss << key << " ";
    }
    return ps_forward_error<ps_db_users_get_t>(kvs_res, ss.str());
  }

  ps_db_users_get_t res;
  for (const auto &lookup_key_res : ps_unwrap(kvs_res)) {
    size_t pos = lookup_key_res.key.rfind('.');
    std::string userid_str = (pos == std::string::npos)
                                 ? lookup_key_res.key
                                 : lookup_key_res.key.substr(pos + 1);

    uid_t userid;
    if (!parse_uid(userid_str, userid)) {
      res.push_back(
          {.userid = userid,
           .result = ps_error(ps_parse_uid_err_t{.uid_str = userid_str})});
      continue;
    }

    if (!ps_is_ok(lookup_key_res.value)) {
      res.push_back({.userid = userid,
                     .result = ps_forward_error<ps_user_t>(
                         lookup_key_res.value,
                         "looking up user " + userid_str + " in flux kvs")});
      continue;
    }

    auto lkrv = lookup_key_res.value;
    auto lookup_key_res_value = ps_unwrap(lkrv);
    double score;
    if (!parse_double(lookup_key_res_value, score)) {
      res.push_back({.userid = userid,
                     .result = ps_error(ps_parse_double_err_t{
                         .double_str = lookup_key_res_value})});
      continue;
    }
    res.push_back({.userid = userid,
                   .result = ps_user_t{.userid = userid, .score = score}});
  }

  return res;
};

ps_result_t<ps_success_t> ps_db_user_set(const ps_user_t &user) {
  auto txn = flux_kvs_txn_create();
  if (!txn) {
    return ps_error(ps_flux_kvs_txn_create_err_t{.msg = strerror(errno)});
  }
  auto key = "priorisea.user." + std::to_string(user.userid);
  auto value = std::to_string(user.score);
  if (flux_kvs_txn_put(txn, 0, key.c_str(), value.c_str()) < 0) {
    return ps_error(ps_flux_kvs_txn_put_err_t{
        .key = key, .value = value, .msg = strerror(errno)});
  }
  return ps_flux_kvs_commit(txn);
}

ps_result_t<ps_success_t>
ps_db_job_update(ps_flux_job_info_t job_info, const ps_formula_t &formula,
                 const std::vector<ps_timewin_t> &timewins) {

  std::map<std::string, double> vars_over;

  auto timewin_str = "timewin_score";
  if (formula.expr.find(timewin_str) != std::string::npos) {
    auto timewin_score = 0;
    bool found = false;
    for (const auto &timewin : timewins) {
      auto cron = cron::make_cron(timewin.cron);
      auto prev_timestamp = ps_cron_prev(cron, job_info.t_submit);
      if (prev_timestamp + timewin.duration > job_info.t_submit) {
        timewin_score += timewin.score;
        found = true;
      }
    }
    if (found) {
      vars_over[timewin_str] = timewin_score;
    }
  }

  auto real_duration = job_info.t_cleanup - job_info.t_run;
  auto duration_ratio = real_duration / job_info.duration;
  if (!std::isinf(duration_ratio)) {
    vars_over["duration_ratio"] = std::min(1.0, duration_ratio);
  }

  auto eval_res = ps_eval_formula(formula, vars_over);
  if (!ps_is_ok(eval_res)) {
    return ps_forward_error<ps_success_t>(
        eval_res, "evaluating formula for job " + std::to_string(job_info.id));
  }

  return ps_db_job_set({.jobid = job_info.id, .score = ps_unwrap(eval_res)});
}

ps_result_t<ps_db_users_get_t> ps_db_users_list() {
  auto ls_res = ps_flux_kvs_ls_dir("priorisea.user");
  if (!ps_is_ok(ls_res)) {
    return ps_forward_error<ps_db_users_get_t>(ls_res,
                                               "listing users in flux kvs");
  }
  ps_db_users_get_t res;
  std::vector<uid_t> userids;
  for (const auto &userid_str : ps_unwrap(ls_res)) {
    uid_t userid;
    if (!parse_uid(userid_str, userid)) {
      res.push_back(
          {.userid = 0,
           .result = ps_error(ps_parse_uid_err_t{.uid_str = userid_str})});
      continue;
    }
    userids.push_back(userid);
  }

  auto get_res = ps_db_users_get(userids);
  if (!ps_is_ok(get_res)) {
    std::stringstream ss;
    ss << "getting users from database: ";
    for (const auto &e : userids) {
      ss << e << " ";
    }
    return ps_forward_error<ps_db_users_get_t>(get_res, ss.str());
  }

  auto get_res_value = ps_unwrap(get_res);
  res.insert(res.end(), get_res_value.begin(), get_res_value.end());
  return res;
}

ps_result_t<ps_db_jobs_get_t> ps_db_jobs_list() {
  auto ls_res = ps_flux_kvs_ls_dir("priorisea.job");
  if (!ps_is_ok(ls_res)) {
    return ps_forward_error<ps_db_jobs_get_t>(ls_res,
                                              "listing jobs from flux kvs");
  }
  auto ls_res_value = ps_unwrap(ls_res);
  std::vector<flux_jobid_t> jobids;
  for (const auto &e : ls_res_value) {
    flux_jobid_t id;
    if (flux_job_id_parse(e.c_str(), &id) < 0) {
      return ps_error(ps_parse_uid_err_t{.uid_str = e},
                      "parsing listed jobs from flux kvs");
    }
    jobids.push_back(id);
  }
  return ps_db_jobs_get(jobids);
}

// TODO: refactor error handling and result
ps_result_t<ps_success_t> ps_db_users_update() {
  // TODO: if a user has not submitted any job in the entire decay period,
  // the score will not be updated. That user will thus keep its current score.
  // until the next time they run a job

  auto get_decay_res = ps_db_decay_get();
  if (!ps_is_ok(get_decay_res)) {
    return ps_forward_error<ps_success_t>(get_decay_res,
                                          "getting decay from database");
  }
  auto decay = ps_unwrap(get_decay_res);

  // timestamp is the latest time possible where decay applies
  auto now = std::chrono::system_clock::now();
  auto timestamp =
      std::chrono::duration<double>(now.time_since_epoch()).count();
  for (const auto &decay_one_obj : decay) {
    timestamp -= decay_one_obj.duration;
  }

  // structure to store sum and count of scores for each
  // user for each time slot of decay
  struct sum_agg {
    double sum;
    size_t count;
  };
  std::map<uid_t, std::vector<sum_agg>> sum_aggs;
  ps_flux_list_inactive_jobs_stream(
      timestamp, INT_MAX, [&sum_aggs, &decay, &timestamp](json_t *jobs) {
        std::vector<flux_jobid_t> jobids;
        struct job_info {
          int decay_index;
          ps_flux_job_info_t info;
        };
        std::map<flux_jobid_t, job_info> jobid_info;

        size_t index;
        json_t *value;
        json_array_foreach(jobs, index, value) {
          auto unpack_res = ps_flux_job_info_unpack(value);
          if (!ps_is_ok(unpack_res)) {
            return ps_forward_error<ps_success_t>(unpack_res,
                                                  "unpacking job info");
          }
          auto unpack_value = ps_unwrap(unpack_res);
          // find the index of the decay step that applies to this job
          auto _timestamp = timestamp;
          int i = decay.size() - 1;
          for (; i >= 0; i--) {
            _timestamp += decay[i].duration;
            if (_timestamp > unpack_value.t_inactive) {
              break;
            }
          }

          jobid_info[unpack_value.id] =
              job_info{.decay_index = i, .info = unpack_value};
        }

        for (const auto &[key, _] : jobid_info) {
          jobids.push_back(key);
        }
        auto get_jobs_res = ps_db_jobs_get(jobids);
        if (!ps_is_ok(get_jobs_res)) {
          std::stringstream ss;
          ss << "getting jobs from database: ";
          for (const auto jobid : jobids) {
            ss << jobid << " ";
          }
          return ps_forward_error<ps_success_t>(get_jobs_res, ss.str());
        }
        auto get_jobs_value = ps_unwrap(get_jobs_res);

        for (const auto &get_job_res : get_jobs_value) {
          if (!ps_is_ok(get_job_res.result)) {
            return ps_forward_error<ps_success_t>(
                get_job_res.result, "getting jobid: '" +
                                        std::to_string(get_job_res.jobid) +
                                        "' from database");
          }
          auto gjv = get_job_res.result;
          auto get_job_value = ps_unwrap(gjv);
          auto job_inf = jobid_info[get_job_res.jobid];
          auto userid = job_inf.info.userid;

          if (sum_aggs.find(userid) == sum_aggs.end()) {
            sum_aggs[userid] = std::vector<sum_agg>(decay.size());
            for (size_t i = 0; i < decay.size(); i++) {
              sum_aggs[userid][i] = {0, 0};
            }
          }

          auto *curr_agg = &sum_aggs[userid][job_inf.decay_index];
          curr_agg->sum +=
              get_job_value.score * decay[job_inf.decay_index].priority;
          curr_agg->count++;
        }
        return (ps_result_t<ps_success_t>)ps_success_t{};
      });

  std::map<uid_t, double> scores;
  for (const auto &[userid, value] : sum_aggs) {
    double score = 0;
    for (const auto &agg : value) {
      if (agg.count > 0) {
        score += agg.sum / agg.count;
      }
    }
    scores[userid] = score;
    auto store_res = ps_db_user_set({.userid = userid, .score = score});
    if (!ps_is_ok(store_res)) {
      return ps_add_context(store_res, "storing user '" +
                                           std::to_string(userid) +
                                           "' in database");
    }
  }
  return ps_flux_plugin_set_score(scores);
}
