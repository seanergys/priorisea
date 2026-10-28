#include "src/daemon/endpoint/error.hpp"

std::string ps_error_message(const ps_plugin_err_t &err) {
  return "Error reaching priority plugin:\n" + err.err;
}

std::string ps_error_message(const ps_job_invalid_t &err) {
  return "Job " + err.jobid + " is invalid. A job must have ran and finished";
}
