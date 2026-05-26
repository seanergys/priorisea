// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include <map>
#include <string>

int run_score(int argc, char *argv[]);
bool validate_score(std::string formula, std::map<std::string, double> &vars);
