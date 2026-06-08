// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "error.hpp"
#include "types.hpp"
#include <string>

std::string ps_decay_to_str(const ps_decay_t &decay);

ps_result_t<ps_decay_t> ps_decay_from_str(const std::string &decay_str);

ps_result_t<ps_success_t> ps_db_decay_set(const ps_decay_t &decay);

ps_result_t<ps_decay_t> ps_db_decay_get();

bool ps_db_is_decay();

ps_result_t<ps_success_t> ps_db_decay_reset();

ps_result_t<ps_success_t> ps_db_decay_init();
