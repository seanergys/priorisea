#pragma once
#include "src/lib/error.hpp"
#include "src/lib/types.hpp"
#include <sqlite3.h>

/**
 * Map of time window name to its get result, one entry per requested name
 */
using ps_db_timewins_get_t = std::map<std::string, ps_result_t<ps_timewin_t>>;
