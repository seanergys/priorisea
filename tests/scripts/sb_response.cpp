// SPDX-FileCopyrightText: 2026 SEANERGYS Partners <https://seanergys.eu>
//
// SPDX-License-Identifier: MIT

// Usage:
//     sb_response IP PORT UUID MESSAGE

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
  // All four arguments are mandatory positionals; there are no options and no
  // defaults, so the caller must always pass IP PORT UUID MESSAGE.
  if (argc < 5) {
    std::cerr << "Usage: " << argv[0] << " IP PORT UUID MESSAGE" << std::endl;
    return -1;
  }

  const std::string ip = argv[1];
  const std::string port = argv[2];
  const std::string uuid = argv[3];
  const std::string reply = argv[4];

  const auto entity = seanerbus::msg::Entity{seanerbus::msg::Uuid{uuid}};

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
