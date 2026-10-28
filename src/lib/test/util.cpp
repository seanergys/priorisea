// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include "src/lib/util.hpp"

#include <catch2/catch.hpp>
#include <climits>
#include <cstdint>

TEST_CASE("parse_int accepts well-formed integers", "[util][parse_int]") {
  int out = 0;
  REQUIRE(parse_int("0", out));
  REQUIRE(out == 0);
  REQUIRE(parse_int("42", out));
  REQUIRE(out == 42);
  REQUIRE(parse_int("-7", out));
  REQUIRE(out == -7);
  REQUIRE(parse_int("+5", out));
  REQUIRE(out == 5);
}

TEST_CASE("parse_int rejects malformed input", "[util][parse_int]") {
  int out = 0;
  REQUIRE_FALSE(parse_int("", out));
  REQUIRE_FALSE(parse_int("abc", out));
  REQUIRE_FALSE(parse_int("42x", out));
  REQUIRE_FALSE(parse_int("99999999999999999999", out));
  REQUIRE_FALSE(parse_int("  ", out));
}

TEST_CASE("parse_double accepts well-formed numbers", "[util][parse_double]") {
  double out = 0.0;
  REQUIRE(parse_double("0", out));
  REQUIRE(out == Approx(0.0));
  REQUIRE(parse_double("3.14", out));
  REQUIRE(out == Approx(3.14));
  REQUIRE(parse_double("1e3", out));
  REQUIRE(out == Approx(1000.0));
  REQUIRE(parse_double("-2.5", out));
  REQUIRE(out == Approx(-2.5));
}

TEST_CASE("parse_double rejects malformed or non-finite input",
          "[util][parse_double]") {
  double out = 0.0;
  REQUIRE_FALSE(parse_double("", out));
  REQUIRE_FALSE(parse_double("abc", out));
  REQUIRE_FALSE(parse_double("1.2x", out));
  REQUIRE_FALSE(parse_double("inf", out));
  REQUIRE_FALSE(parse_double("nan", out));
}

TEST_CASE("parse_uint64 accepts well-formed values", "[util][parse_uint64]") {
  uint64_t out = 0;
  REQUIRE(parse_uint64("0", out));
  REQUIRE(out == 0);
  REQUIRE(parse_uint64("42", out));
  REQUIRE(out == 42);
  REQUIRE(parse_uint64("1000", out));
  REQUIRE(out == 1000);

  // The full unsigned 64-bit range is accepted.
  REQUIRE(parse_uint64("18446744073709551615", out));
  REQUIRE(out == ULLONG_MAX);
}

TEST_CASE("parse_uint64 rejects malformed or overflowing input",
          "[util][parse_uint64]") {
  uint64_t out = 0;
  REQUIRE_FALSE(parse_uint64("", out));
  REQUIRE_FALSE(parse_uint64("abc", out));
  REQUIRE_FALSE(parse_uint64("42x", out));
  // UINT64_MAX + 1 overflows.
  REQUIRE_FALSE(parse_uint64("18446744073709551616", out));
}

TEST_CASE("parse_uid accepts non-negative uids", "[util][parse_uid]") {
  uid_t out = 0;
  REQUIRE(parse_uid("0", out));
  REQUIRE(out == 0);
  REQUIRE(parse_uid("1000", out));
  REQUIRE(out == 1000);
}

TEST_CASE("parse_uid rejects negatives, overflow and garbage",
          "[util][parse_uid]") {
  uid_t out = 0;
  REQUIRE_FALSE(parse_uid("-1", out));
  REQUIRE_FALSE(parse_uid("abc", out));
  // UINT_MAX + 1 overflows uid_t even when it fits in unsigned long.
  REQUIRE_FALSE(parse_uid("4294967296", out));
}
