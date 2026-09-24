// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#pragma once
#include "lib/error.hpp"
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

template <typename T> struct ps_column_t {
  std::string header;
  std::function<std::string(const T &, std::size_t)> getter;
};

/**
 * Prints a vertical table of rows and columns to the console.
 *
 * @param rows The rows of the table.
 * @param columns The columns of the table.
 */
template <typename T>
void ps_print_table_v(const std::vector<T> &rows,
                      const std::vector<ps_column_t<T>> &columns) {
  std::vector<std::size_t> widths;

  // Compute column widths
  for (const auto &col : columns) {
    std::size_t width = col.header.size();

    for (std::size_t i = 0; i < rows.size(); ++i) {
      width = std::max(width, col.getter(rows[i], i).size());
    }

    widths.push_back(width);
  }

  // Print header
  for (std::size_t i = 0; i < columns.size(); ++i) {
    std::cout << std::left << std::setw(widths[i]) << columns[i].header;

    if (i + 1 < columns.size())
      std::cout << " | ";
  }

  std::cout << '\n';

  // Print separator
  for (std::size_t i = 0; i < columns.size(); ++i) {
    std::cout << std::string(widths[i], '-');

    if (i + 1 < columns.size())
      std::cout << "-+-";
  }

  std::cout << '\n';

  // Print rows
  for (std::size_t row = 0; row < rows.size(); ++row) {
    for (std::size_t col = 0; col < columns.size(); ++col) {
      std::cout << std::left << std::setw(widths[col])
                << columns[col].getter(rows[row], row);

      if (col + 1 < columns.size())
        std::cout << " | ";
    }

    std::cout << '\n';
  }

  std::cout << std::endl;
}

/**
 * Prints a horizontal table of rows and columns to the console.
 *
 * @param rows The rows of the table.
 * @param columns The columns of the table.
 */
template <typename T>
void ps_print_table_h(const std::vector<T> &rows,
                      const std::vector<ps_column_t<T>> &columns) {
  std::vector<std::size_t> widths;

  // Width of the leading header column
  std::size_t header_width = 0;
  for (const auto &col : columns) {
    header_width = std::max(header_width, col.header.size());
  }
  widths.push_back(header_width);

  // Width of each record column (widest value in that record)
  for (std::size_t row = 0; row < rows.size(); ++row) {
    std::size_t width = 0;
    for (const auto &col : columns) {
      width = std::max(width, col.getter(rows[row], row).size());
    }
    widths.push_back(width);
  }

  // One line per field
  for (std::size_t col = 0; col < columns.size(); ++col) {
    std::cout << std::left << std::setw(widths[0]) << columns[col].header;

    for (std::size_t row = 0; row < rows.size(); ++row) {
      std::cout << " | " << std::left << std::setw(widths[row + 1])
                << columns[col].getter(rows[row], row);
    }

    std::cout << '\n';
  }

  std::cout << std::endl;
}

/**
 * Prints an error message to the console.
 *
 * @param err The error to print.
 */
void cli_error(const ps_error_t &err);

/**
 * Prints an error message to the console.
 *
 * @param res The result to print.
 */
template <typename T> void cli_error(const ps_result_t<T> &res) {
  auto err = ps_unwrap_error(res);
  cli_error(err);
}
