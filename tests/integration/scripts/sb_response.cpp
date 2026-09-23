// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

// Usage:
//     sb_response [--ip IP] [--port PORT] UUID MESSAGE

#include <seanerbus/client/connection.hpp>
#include <seanerbus/client/simple_event_loop.hpp>
#include <seanerbus/msg/message.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace {
void res_handler(
    seanerbus::client::Connection &conn,
    seanerbus::Expected<seanerbus::msg::Message, seanerbus::Error> res,
    const std::string &reply) {
  if (!res.ok()) {
    std::cerr << "Failed to listen for requests: " << res.unwrap_error().what()
              << std::endl;
    conn.shutdown(seanerbus::Unexpected(res.unwrap_error()));
    return;
  }

  auto msg = res.unwrap_ok();

  if (msg.payload_type() != HelloWorldReqV1::TYPE) {
    std::cerr << "Got something else than a HelloWorldReqV1, ignoring!"
              << std::endl;
    return;
  }

  auto rd_res = seanerbus::msg::MessageReader<HelloWorldReqV1>::from_msg(msg);
  if (!rd_res.ok()) {
    std::cerr << "Could not parse message as HelloWorldReqV1: "
              << rd_res.unwrap_error().what() << std::endl;
    return;
  }
  auto rd = rd_res.unwrap_ok();

  std::cout << "Got incoming HelloWorldReqV1: " << rd.reader().getMsg().cStr()
            << std::endl;

  auto bd = seanerbus::msg::MessageBuilder<HelloWorldResV1>();

  bd.builder().setMsg(reply);

  conn.respond_to(msg, bd.to_message(msg.entity()));
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

  const std::string reply = positionals[1];

  // Connect to the server.
  const std::string endpoint = "demobus://" + ip + ":" + port;
  seanerbus::client::Connection conn(endpoint, endpoint);

  conn.register_endpoint(
      entity,
      [&](seanerbus::client::Connection &c,
          seanerbus::Expected<seanerbus::msg::Message, seanerbus::Error> res) {
        res_handler(c, res, reply);
      });

  seanerbus::client::SimpleEventLoop sel(conn);
  sel.run();

  if (!conn.result().ok()) {
    std::cerr << "Error while running sb_response: "
              << conn.result().get_error().what() << std::endl;
    return 1;
  }

  return 0;
}
