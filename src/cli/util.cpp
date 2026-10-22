// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/cli/util.hpp"
#include "src/lib/formula.hpp"

void ps_print_formula(const ps_formula_t &formula, bool json) {
  if (json) {
    auto obj = ps_formula_to_json(formula);
    char *dump = json_dumps(obj, JSON_INDENT(2));
    if (dump) {
      std::cout << dump << std::endl;
      free(dump);
    }
    json_decref(obj);
    return;
  }
  ps_print_table_v(std::vector<ps_formula_t>{formula},
                   {{"Name", [](auto &f, auto) { return f.name; }},
                    {"Expression", [](auto &f, auto) { return f.expr; }},
                    {"Defaults", [](auto &value, auto index) -> auto {
                       std::stringstream ss;
                       bool first = true;
                       for (const auto &[name, value] : value.defaults) {
                         if (!first) {
                           ss << ", ";
                         } else {
                           first = false;
                         }
                         ss << name << "=" << value;
                       }
                       return ss.str();
                     }}});
}
