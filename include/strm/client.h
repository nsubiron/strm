#pragma once

#include "strm/detail/strm_asio/udp_client.h"

#include <string>

namespace strm {

  class client : public strm::detail::strm_asio::udp_client {
  public:

    using underlying_client = strm::detail::strm_asio::udp_client;
    using endpoint = underlying_client::endpoint;

    explicit client(boost::asio::io_service &io_service, endpoint ep)
      : underlying_client(io_service, std::move(ep)) {}

    explicit client(boost::asio::io_service &io_service, const std::string &host, uint16_t port)
      : client(io_service, endpoint(boost::asio::ip::address::from_string(host), port)) {}

    explicit client(boost::asio::io_service &io_service, uint16_t port)
      : client(io_service, "127.0.0.1", port) {}
  };

} // namespace strm

