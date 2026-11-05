#pragma once
#include "src/lib/types.hpp"
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

/**
 * A table column with a header and a value getter
 */
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
 * Print a formula to the console as a table or JSON
 *
 * @param formula Formula to print
 * @param json Whether to print the formula as JSON
 */
void ps_print_formula(const ps_formula_t &formula, bool json);

std::string
ps_formula_vars_to_string(const std::map<std::string, double> variables);

void print_jobs_table(const std::vector<ps_job_t> &jobs, bool plain);
