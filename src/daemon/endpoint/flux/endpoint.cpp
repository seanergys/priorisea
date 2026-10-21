// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/daemon/endpoint/flux/endpoint.hpp"
#include "src/daemon/endpoint/flux/callback/formula.hpp"
#include "src/daemon/endpoint/flux/callback/info.hpp"
#include "src/daemon/endpoint/flux/callback/job.hpp"
#include "src/daemon/endpoint/flux/callback/plugin.hpp"
#include "src/daemon/endpoint/flux/callback/timewin.hpp"
#include "src/daemon/endpoint/flux/callback/user.hpp"
#include "src/lib/error.hpp"
#include <csignal>
#include <cstring>
#include <deque>
#include <flux/core.h>

struct ps_daemon_endpoint_flux_t {
  const char *topic;
  int (*handle)(const ps_daemon_endpoint_flux_callback_args_t &);
};

struct ps_daemon_endpoint_flux_args_t {
  int (*handle)(const ps_daemon_endpoint_flux_callback_args_t &);
  sqlite3 *db;
  std::string seanerbus_url;
};

static const std::vector<ps_daemon_endpoint_flux_t> endpoints = {
    {"priorisea.daemon.formula.get", ps_daemon_endpoint_flux_formula_get},
    {"priorisea.daemon.formula.list", ps_daemon_endpoint_flux_formula_list},
    {"priorisea.daemon.job.get", ps_daemon_endpoint_flux_job_get},
    {"priorisea.daemon.job.list", ps_daemon_endpoint_flux_job_list},
    {"priorisea.daemon.timewin.get", ps_daemon_endpoint_flux_timewin_get},
    {"priorisea.daemon.timewin.list", ps_daemon_endpoint_flux_timewin_list},
    {"priorisea.daemon.user.get", ps_daemon_endpoint_flux_user_get},
};

static const std::vector<ps_daemon_endpoint_flux_t> endpoints_priviledged = {
    {"priorisea.daemon.formula.update", ps_daemon_endpoint_flux_formula_update},
    {"priorisea.daemon.formula.reset", ps_daemon_endpoint_flux_formula_reset},
    {"priorisea.daemon.info.get", ps_daemon_endpoint_flux_info_get},
    {"priorisea.daemon.job.update", ps_daemon_endpoint_flux_job_update},
    {"priorisea.daemon.plugin.sync", ps_daemon_endpoint_flux_plugin_sync},
    {"priorisea.daemon.plugin.debug", ps_daemon_endpoint_flux_plugin_debug},
    {"priorisea.daemon.timewin.update", ps_daemon_endpoint_flux_timewin_update},
    {"priorisea.daemon.timewin.create", ps_daemon_endpoint_flux_timewin_create},
    {"priorisea.daemon.timewin.delete", ps_daemon_endpoint_flux_timewin_delete},
    {"priorisea.daemon.user.list", ps_daemon_endpoint_flux_user_list},
    {"priorisea.daemon.user.update", ps_daemon_endpoint_flux_user_update}};

static void ps_daemon_endpoint_flux_signal_cb(flux_reactor_t *r,
                                              flux_watcher_t *w, int revents,
                                              void *arg) {
  auto h = static_cast<flux_t *>(arg);
  int signum = flux_signal_watcher_get_signum(w);
  flux_log(h, LOG_INFO, "%s: received signal %d, stopping daemon", __FUNCTION__,
           signum);
  flux_reactor_stop(r);
}

static void ps_daemon_endpoint_flux_dispatch_cb(flux_t *h,
                                                flux_msg_handler_t *mh,
                                                const flux_msg_t *msg,
                                                void *arg) {
  (void)mh;
  const char *topic = nullptr;
  flux_msg_get_topic(msg, &topic);
  const auto *ep = static_cast<const ps_daemon_endpoint_flux_args_t *>(arg);
  auto args = ps_daemon_endpoint_flux_callback_args_t{
      .h = h, .msg = msg, .db = ep->db, .seanerbus_url = ep->seanerbus_url};
  ep->handle(args);
}

static std::vector<flux_msg_handler_t *>
ps_daemon_endpoint_flux_create_handlers(
    flux_t *handle, const std::vector<ps_daemon_endpoint_flux_t> &endpoints,
    bool priviledged, sqlite3 *db) {
  static std::deque<ps_daemon_endpoint_flux_args_t> args;
  std::vector<flux_msg_handler_t *> res;
  for (const auto &ep : endpoints) {
    auto match =
        flux_match_init(FLUX_MSGTYPE_REQUEST, FLUX_MATCHTAG_NONE, ep.topic);
    args.push_back(
        ps_daemon_endpoint_flux_args_t{.handle = ep.handle, .db = db});
    auto *mh = flux_msg_handler_create(
        handle, match, ps_daemon_endpoint_flux_dispatch_cb, &args.back());
    if (!mh) {
      flux_log_error(handle, "ps_daemon: failed to create handler for '%s'",
                     ep.topic);
      continue;
    }
    if (!priviledged) {
      flux_msg_handler_allow_rolemask(mh, FLUX_ROLE_USER);
    }
    flux_msg_handler_start(mh);
    res.push_back(mh);
  }
  return res;
}

ps_result_t<ps_success_t> ps_daemon_endpoint_flux_run(sqlite3 *db,
                                                      std::string seanerbus) {
  auto *h = flux_open(NULL, 0);
  if (!h) {
    return ps_error(ps_flux_open_err_t{.strerror = strerror(errno)});
  }

  auto *f = flux_service_register(h, "priorisea");
  if (!f) {
    auto err = "flux service register failed: " + std::string(strerror(errno));
    return ps_error(ps_unexpected_err_t{.msg = err});
  }
  if (flux_rpc_get(f, nullptr) < 0) {
    auto err = std::string(flux_future_error_string(f));
    flux_future_destroy(f);
    return ps_error(ps_unexpected_err_t{.msg = err});
  }
  flux_future_destroy(f);

  auto *reactor = flux_get_reactor(h);

  auto *sigterm_w = flux_signal_watcher_create(
      reactor, SIGTERM, ps_daemon_endpoint_flux_signal_cb, h);
  auto *sigint_w = flux_signal_watcher_create(
      reactor, SIGINT, ps_daemon_endpoint_flux_signal_cb, h);
  flux_watcher_start(sigterm_w);
  flux_watcher_start(sigint_w);

  // Create message handlers
  std::vector<flux_msg_handler_t *> handlers;
  auto mh = ps_daemon_endpoint_flux_create_handlers(h, endpoints, false, db);
  auto mhp = ps_daemon_endpoint_flux_create_handlers(h, endpoints_priviledged,
                                                     true, db);
  handlers.insert(handlers.end(), mh.begin(), mh.end());
  handlers.insert(handlers.end(), mhp.begin(), mhp.end());

  // Run the reactor
  flux_reactor_run(reactor, 0);

  // Teardown
  for (auto *mh : handlers) {
    flux_msg_handler_stop(mh);
    flux_msg_handler_destroy(mh);
  }
  flux_watcher_stop(sigint_w);
  flux_watcher_stop(sigterm_w);
  flux_watcher_destroy(sigint_w);
  flux_watcher_destroy(sigterm_w);
  flux_close(h);

  return ps_success_t{};
}
