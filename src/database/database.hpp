#pragma once
#include "src/lib/error.hpp"
#include <string>

/**
 * Atomically creates the database: the schema, the default formulas, and the
 * info row are written in a single transaction
 *
 * @param path The path of the database to create
 * @param user_period The default user_period
 * @return A result indicating success or failure
 */
ps_result_t<ps_success_t> ps_db_database_create(const std::string &path,
                                                double user_period);
