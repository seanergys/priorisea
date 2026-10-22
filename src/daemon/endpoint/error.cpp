#include "src/daemon/endpoint/error.hpp"

std::string ps_error_message(const ps_plugin_err_t &err) {
  return "Error reaching priority plugin:\n" + err.err;
}
