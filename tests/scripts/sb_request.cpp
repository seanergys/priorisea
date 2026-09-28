// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

// Usage:
//     sb_request [--ip IP] [--port PORT] UUID MESSAGE

#include <seanerbus/client/connection.hpp>
#include <seanerbus/client/simple_event_loop.hpp>
#include <seanerbus/msg/message.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace {
void req_handler(
    seanerbus::client::Connection &conn,
    seanerbus::Expected<seanerbus::msg::Message, seanerbus::Error> res) {
  if (!res.ok()) {
    std::cerr << "Failed to receive response: " << res.unwrap_error().what()
              << std::endl;
    conn.shutdown(seanerbus::Unexpected(res.unwrap_error()));
    return;
  }

  auto msg = std::move(res.unwrap_ok());

  if (msg.payload_type() != HelloWorldResV1::TYPE) {
    std::cerr << "Got unknown return message type!" << std::endl;
    conn.shutdown();
    return;
  }

  auto rd_res = seanerbus::msg::MessageReader<HelloWorldResV1>::from_msg(msg);
  if (!rd_res.ok()) {
    std::cerr << "Failed to decode response as HelloWorldResV1: "
              << rd_res.unwrap_error().what() << std::endl;
    conn.shutdown();
    return;
  }

  auto rd = std::move(rd_res.unwrap_ok());

  std::cout << "Got answer to request: " << rd.reader().getMsg().cStr()
            << std::endl;

  conn.shutdown();
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

  if (positionals.size() < 2) {
    std::cerr << "Usage: " << argv[0] << " [--ip IP] [--port PORT] UUID MESSAGE"
              << std::endl;
    return -1;
  }

  const auto entity =
      seanerbus::msg::Entity{seanerbus::msg::Uuid{positionals[0]}};

  const std::string message = positionals[1];

  // Connect to the server.
  const std::string endpoint = "demobus://" + ip + ":" + port;
  seanerbus::client::Connection conn(endpoint, endpoint);

  seanerbus::msg::MessageBuilder<HelloWorldReqV1> bd;

  bd.builder().setMsg(message);

  conn.request(bd.to_message(entity), req_handler);

  seanerbus::client::SimpleEventLoop sel(conn);
  sel.run();

  if (!conn.result().ok()) {
    std::cerr << "sb_request failed: " << conn.result().get_error().what()
              << std::endl;
    return 1;
  }

  return 0;
}
