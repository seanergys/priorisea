// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/lib/formula.hpp"
#include "src/lib/error.hpp"

#include <catch2/catch.hpp>

TEST_CASE("ps_validate_formula accepts well-formed input",
          "[formula][ps_validate_formula]") {
  // A plain arithmetic expression.
  REQUIRE(ps_is_ok(ps_validate_formula({.name = "a", .expr = "1 + 2"})));

  // Variables provided through defaults.
  REQUIRE(ps_is_ok(
      ps_validate_formula({.name = "b",
                           .expr = "<var:x> * 0.5 + <var:y> * 0.5",
                           .defaults = {{"var:x", 1}, {"var:y", 2}}})));

  // The shipped default formulas must validate.
  REQUIRE(ps_is_ok(ps_validate_formula(
      {.name = "job_score",
       .expr = "<var:duration_ratio> + <var:timewin_score>"})));
}

TEST_CASE("ps_validate_formula rejects malformed input",
          "[formula][ps_validate_formula]") {
  // Trailing operator.
  REQUIRE_FALSE(ps_is_ok(ps_validate_formula({.name = "bad", .expr = "1 +"})));

  // Unbalanced parenthesis.
  REQUIRE_FALSE(
      ps_is_ok(ps_validate_formula({.name = "bad", .expr = "(1 + 2"})));

  // Garbage that tinyexpr cannot parse.
  REQUIRE_FALSE(ps_is_ok(ps_validate_formula({.name = "bad", .expr = "@#$"})));
}

TEST_CASE("ps_eval_formula accepts well-formed input",
          "[formula][ps_eval_formula]") {
  // A constant expression.
  auto r1 = ps_eval_formula({.name = "c", .expr = "1 + 2"}, {});
  REQUIRE(ps_is_ok(r1));
  REQUIRE(ps_unwrap(r1) == Approx(3.0));

  // Variables supplied explicitly.
  auto r2 = ps_eval_formula({.name = "v", .expr = "<var:x> + <var:y>"},
                            {{"var:x", 3.0}, {"var:y", 4.0}});
  REQUIRE(ps_is_ok(r2));
  REQUIRE(ps_unwrap(r2) == Approx(7.0));

  // Defaults fill in the variables.
  auto r3 = ps_eval_formula({.name = "d",
                             .expr = "<var:x> * 0.5 + <var:y> * 0.5",
                             .defaults = {{"var:x", 10.0}, {"var:y", 20.0}}},
                            {});
  REQUIRE(ps_is_ok(r3));
  REQUIRE(ps_unwrap(r3) == Approx(15.0));

  // Explicit variables override the defaults.
  auto r4 = ps_eval_formula({.name = "d",
                             .expr = "<var:x> * 0.5 + <var:y> * 0.5",
                             .defaults = {{"var:x", 10.0}, {"var:y", 20.0}}},
                            {{"var:x", 100.0}, {"var:y", 200.0}});
  REQUIRE(ps_is_ok(r4));
  REQUIRE(ps_unwrap(r4) == Approx(150.0));
}

TEST_CASE("ps_eval_formula rejects malformed input",
          "[formula][ps_eval_formula]") {
  auto r = ps_eval_formula({.name = "bad", .expr = "1 +"}, {});
  REQUIRE_FALSE(ps_is_ok(r));
  REQUIRE(ps_is_error_type<ps_formula_eval_err_t>(r));
}

TEST_CASE("ps_formula_from_json accepts well-formed input",
          "[formula][ps_formula_from_json]") {
  json_t *obj = json_pack("{s:s,s:s,s:{s:f}}", "name", "f", "expr", "1 + 2",
                          "defaults", "var:x", 1.5);
  auto res = ps_formula_from_json(obj);
  REQUIRE(ps_is_ok(res));
  REQUIRE(ps_unwrap(res).name == "f");
  REQUIRE(ps_unwrap(res).expr == "1 + 2");
  REQUIRE(ps_unwrap(res).defaults.size() == 1);
  REQUIRE(ps_unwrap(res).defaults.at("var:x") == Approx(1.5));
  json_decref(obj);

  // Defaults are optional.
  json_t *obj2 = json_pack("{s:s,s:s}", "name", "f", "expr", "1 + 2");
  auto res2 = ps_formula_from_json(obj2);
  REQUIRE(ps_is_ok(res2));
  REQUIRE(ps_unwrap(res2).defaults.empty());
  json_decref(obj2);
}

TEST_CASE("ps_formula_from_json rejects malformed input",
          "[formula][ps_formula_from_json]") {
  // Missing name.
  json_t *o1 = json_pack("{s:s}", "expr", "1 + 2");
  auto r1 = ps_formula_from_json(o1);
  REQUIRE_FALSE(ps_is_ok(r1));
  REQUIRE(ps_is_error_type<ps_field_missing_t>(r1));
  json_decref(o1);

  // Name present but not a string.
  json_t *o2 = json_pack("{s:i,s:s}", "name", 42, "expr", "1 + 2");
  auto r2 = ps_formula_from_json(o2);
  REQUIRE_FALSE(ps_is_ok(r2));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r2));
  json_decref(o2);

  // Missing expr.
  json_t *o3 = json_pack("{s:s}", "name", "f");
  auto r3 = ps_formula_from_json(o3);
  REQUIRE_FALSE(ps_is_ok(r3));
  REQUIRE(ps_is_error_type<ps_field_missing_t>(r3));
  json_decref(o3);

  // Expr present but not a string.
  json_t *o4 = json_pack("{s:s,s:i}", "name", "f", "expr", 42);
  auto r4 = ps_formula_from_json(o4);
  REQUIRE_FALSE(ps_is_ok(r4));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r4));
  json_decref(o4);

  // Defaults present but not an object.
  json_t *o5 = json_pack("{s:s,s:s,s:s}", "name", "f", "expr", "1 + 2",
                         "defaults", "defaults");
  auto r5 = ps_formula_from_json(o5);
  REQUIRE_FALSE(ps_is_ok(r5));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r5));
  json_decref(o5);

  // Defaults present but value is not a float.
  json_t *o6 = json_pack("{s:s,s:s,s:{s:s}}", "name", "f", "expr", "1 + 2",
                         "defaults", "var", "value");
  auto r6 = ps_formula_from_json(o6);
  REQUIRE_FALSE(ps_is_ok(r6));
  REQUIRE(ps_is_error_type<ps_field_type_mismatch_t>(r6));
  json_decref(o6);
}

TEST_CASE("ps_formula_to_json and ps_formula_from_json are inverse",
          "[formula]") {
  ps_formula_t formula = {
      .name = "job_score",
      .expr = "<var:duration_ratio> + <var:timewin_score>",
      .defaults = {{"var:duration_ratio", 0}, {"var:timewin_score", 0}}};
  json_t *j = ps_formula_to_json(formula);
  REQUIRE(json_is_object(j));
  auto res = ps_formula_from_json(j);
  REQUIRE(ps_is_ok(res));
  auto back = ps_unwrap(res);
  REQUIRE(back.name == formula.name);
  REQUIRE(back.expr == formula.expr);
  REQUIRE(back.defaults.size() == formula.defaults.size());
  REQUIRE(back.defaults.at("var:duration_ratio") == Approx(0.0));
  json_decref(j);
}
