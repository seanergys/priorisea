#include <cstdint>
#include <cstdlib>
#include <flux/core.h>
#include <flux/core/plugin.h>
#include <flux/jobtap.h>
#include <fmt/core.h>
#include <jansson.h>
#include <string.h>

static int priority_cb(flux_plugin_t *p, const char *topic,
                       flux_plugin_arg_t *args, void *data) {

  json_t *jobspec, *entry, *end_event;
  int64_t jobid, priority;
  int state, prev_state, userid, urgency;
  float t_submit;

  flux_t *h = flux_jobtap_get_flux(p);
  if (flux_plugin_arg_unpack(args, FLUX_PLUGIN_ARG_IN, "{s:o s:I s:i s:i s:i s:i s:I s:f s:o}",
      "jobspec", &jobspec,
      "id", &jobid,
      "state", &state,
      "prev_state", &prev_state,
      "userid", &userid,
      "urgency", &urgency,
      "priority", &priority,
      "t_submit", &t_submit,
      "entry", &entry) < 0) {
    flux_log_error(h, "%s", flux_plugin_arg_strerror(args));
    return -1;
  }

  int64_t priority_out;
  bool priority_unavail = false;
  if (flux_plugin_arg_unpack(args, FLUX_PLUGIN_ARG_OUT, "{s:I}",
      "priority", &priority_out) < 0) {
          priority_out = flux_jobtap_priority_unavail(p, args);
          priority_unavail = true;
  }

  flux_log(h, LOG_DEBUG, "SEA_DEBUG -----");
  flux_log(h, LOG_DEBUG, "ARG_IN: jobid=%ld state=%d prev_state=%d userid=%d urgency=%d priority=%ld t_submit=%f ",
      jobid, state, prev_state, userid, urgency, priority, t_submit);
  flux_log(h, LOG_DEBUG, "ARG_OUT: priority=%ld priority_unavail=%d", priority_out, priority_unavail);

  if (flux_plugin_arg_pack(args, FLUX_PLUGIN_ARG_OUT, "{s:I}", "priority",
                           priority_out) < 0) {
    flux_log_error(h, "%s", flux_plugin_arg_strerror(args));
    return -1;
  }


  return 0;
}

static const struct flux_plugin_handler tab[] = {
    {"job.state.priority", priority_cb, NULL},
    {"job.priority.get", priority_cb, NULL},
    {0},
};

extern "C" int flux_plugin_init(flux_plugin_t *p) {
  flux_t *h = flux_jobtap_get_flux(p);

  if (flux_plugin_register(p, "sea_debug", tab) < 0)
    return -1;

  return 0;
}
