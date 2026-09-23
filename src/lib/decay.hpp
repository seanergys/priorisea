// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "error.hpp"
#include "types.hpp"
#include <jansson.h>
#include <string>

json_t *ps_decay_frame_to_json(const ps_decay_frame_t &frame);
ps_decay_frame_t ps_decay_frame_from_json(json_t *obj);

json_t *ps_decay_to_json(const ps_decay_t &decay);
ps_decay_t ps_decay_from_json(json_t *obj);
ps_result_t<ps_decay_t> ps_decay_from_string(const std::string &expr);

ps_result_t<ps_success_t> ps_db_decay_set(const ps_decay_t &decay);

ps_result_t<ps_decay_t> ps_db_decay_get();

bool ps_db_is_decay();

ps_result_t<ps_success_t> ps_db_decay_reset();

ps_result_t<ps_success_t> ps_db_decay_init();
