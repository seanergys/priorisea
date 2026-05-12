#include <algorithm>
#include <functional>
#include <iomanip>
// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

#include <iostream>
#include <string>
#include <vector>

template <typename T> struct column {
  std::string header;
  std::function<std::string(const T &, std::size_t)> getter;
};

template <typename T>
void print_table(const std::vector<T> &rows,
                 const std::vector<column<T>> &columns) {
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
