#pragma once
#include <flux/core.h>
#include <string>

/**
 * Error reaching the priority plugin
 */
struct ps_plugin_err_t {
  std::string err;
};

/**
 * Renders the priority plugin error into a human readable message
 *
 * @param err The plugin error
 * @return The formatted error message
 */
std::string ps_error_message(const ps_plugin_err_t &err);
