// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

// Usage:
//     sb_resolve [--ip IP] [--port PORT] key=value [key=value ...]

#include <seanerbus/client/connection.hpp>
#include <seanerbus/client/simple_event_loop.hpp>
#include <seanerbus/helpers/resolve.hpp>

#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
std::map<std::string, std::string>
parse_name(const std::vector<std::string> &args) {
  std::map<std::string, std::string> name;

  for (const auto &arg : args) {
    const auto pos = arg.find('=');
    if (pos == std::string::npos) {
      throw std::runtime_error("expected key=value, got: " + arg);
    }
    name.emplace(arg.substr(0, pos), arg.substr(pos + 1));
  }

  return name;
}
} // namespace

int main(int argc, char **argv) {
  std::string ip = "localhost";
  std::string port = "5398";
  std::vector<std::string> positionals;

  for (int i = 1; i < argc; ++i) {
    const std::string arg(argv[i]);
    if (arg == "--ip" && i + 1 < argc) {
      ip = argv[++i];
    } else if (arg == "--port" && i + 1 < argc) {
      port = argv[++i];
    } else {
      positionals.push_back(arg);
    }
  }

  if (positionals.empty()) {
    std::cerr << "Usage: " << argv[0]
              << " [--ip IP] [--port PORT] key=value [key=value ...]"
              << std::endl;
    return -1;
  }

  std::map<std::string, std::string> name;
  try {
    name = parse_name(positionals);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }

  // Connect to the server.
  const std::string endpoint = "demobus://" + ip + ":" + port;
  seanerbus::client::Connection conn(endpoint, endpoint);

  bool matched = false;
  seanerbus::resolve::resolve(
      conn, name,
      [&](seanerbus::client::Connection &c,
          seanerbus::Expected<seanerbus::resolve::ResolveResult,
                              seanerbus::Error>
              res) {
        if (!res.ok()) {
          c.shutdown(seanerbus::Unexpected(res.unwrap_error()));
          return;
        }

        auto result = res.unwrap_ok();
        for (const auto &entity : result.entities()) {
          std::cout << entity.as_string() << std::endl;
          matched = true;
        }

        c.shutdown();
      });

  seanerbus::client::SimpleEventLoop sel(conn);
  sel.run();

  if (!conn.result().ok()) {
    std::cerr << "Failed to resolve service name: "
              << conn.result().get_error().what() << std::endl;
    return 1;
  }

  if (!matched) {
    std::cerr << "No service matched the given name" << std::endl;
    return 1;
  }

  return 0;
}
