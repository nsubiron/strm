#pragma once

#include "detail/async_udp_client.h"

#include <string>

namespace strm {

  class client : public detail::async_udp_client {
  public:

    using endpoint = detail::async_udp_client::endpoint;

    explicit client(boost::asio::io_service &io_service, endpoint ep)
      : detail::async_udp_client(io_service, std::move(ep)) {}

    explicit client(boost::asio::io_service &io_service, const std::string &host, uint16_t port)
      : client(io_service, endpoint(boost::asio::ip::address::from_string(host), port)) {}

    explicit client(boost::asio::io_service &io_service, uint16_t port)
      : client(io_service, "127.0.0.1", port) {}
  };

} // namespace strm

