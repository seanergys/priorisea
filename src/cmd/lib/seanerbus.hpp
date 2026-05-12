#include <schemas/msg.capnp.h>
#include <seanerbus_client/connection.hpp>
#include <seanerbus_client/simple_event_loop.hpp>
#include <seanerbus_msg/hello_world_req_v1.hpp>
#include <string>
#include <unistd.h>

enum SEANERBUS_ERR {
  SEANERBUS_ERR_OK,
  SEANERBUS_ERR_CONNECTION_FAILED,
  SEANERBUS_ERR_CLIENT_FAILED
};

template <class F, class... Args>
enum SEANERBUS_ERR
sb_request(std::string ip, std::string port, std::string topic,
           std::pair<uint64_t, std::vector<std::byte>> payload, F &&f,
           Args &&...args) {
  auto addr = seanerbus::Address{seanerbus::Uuid{topic}};
  try {
    seanerbus::client::Connection conn(ip, port);
    conn.request(addr, payload, f, args...);
    seanerbus::client::SimpleEventLoop sel(conn);
    try {
      sel.run();
    } catch (const std::exception &e) {
      return SEANERBUS_ERR_CLIENT_FAILED;
    }
  } catch (const std::exception &e) {
    return SEANERBUS_ERR_CONNECTION_FAILED;
  }
  return SEANERBUS_ERR_OK;
}
