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

std::string
ps_formula_vars_to_string(const std::map<std::string, double> variables) {
  std::string result;
  for (const auto &[var, val] : variables) {
    if (!result.empty()) {
      result += ", ";
    }
    result += var + "=" + std::to_string(val);
  }
  return result;
}

void print_jobs_table(const std::vector<ps_job_t> &jobs, bool plain) {
  if (plain) {
    ps_print_table_v(
        jobs, {{"Job ID", [](auto &j, auto) { return j.jobid; }},
               {"User ID",
                [](auto &j, auto) {
                  return std::to_string(static_cast<uint64_t>(j.userid));
                }},
               {"t_inactive (s)",
                [](auto &j, auto) { return std::to_string(j.t_inactive); }},
               {"Score", [](auto &j, auto) { return std::to_string(j.score); }},
               {"Duration (s)",
                [](auto &j, auto) { return std::to_string(j.duration); }}});
  } else {
    ps_print_table_v(
        jobs,
        {{"Job ID", [](auto &j, auto) { return j.jobid; }},
         {"User ID",
          [](auto &j, auto) {
            return std::to_string(static_cast<uint64_t>(j.userid));
          }},
         {"t_inactive (s)",
          [](auto &j, auto) { return std::to_string(j.t_inactive); }},
         {"Score", [](auto &j, auto) { return std::to_string(j.score); }},
         {"Duration (s)",
          [](auto &j, auto) { return std::to_string(j.duration); }},
         {"Score formula", [](auto &j, auto) { return j.s_expr; }},
         {"Score defaults",
          [](auto &j, auto) {
            return ps_formula_vars_to_string(j.s_defaults);
          }},
         {"Score variables",
          [](auto &j, auto) { return ps_formula_vars_to_string(j.s_values); }},
         {"Weight",
          [](auto &j, auto) {
            if (j.weight) {
              return std::to_string(*j.weight);
            } else {
              return std::string("/");
            }
          }},
         {"Weight variables", [](auto &j, auto) {
            if (j.w_values) {
              return ps_formula_vars_to_string(*j.w_values);
            } else {
              return std::string("/");
            }
          }}});
  }
}
