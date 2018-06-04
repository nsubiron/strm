#pragma once

#include "detail/async_udp_server.h"
#include "detail/dispatcher.h"

#include "stream.h"

#include <memory>
#include <mutex>

namespace strm {

  class server {
  public:

    using endpoint = detail::async_udp_server::endpoint;

    explicit server(boost::asio::io_service &io_service, endpoint ep);

    explicit server(boost::asio::io_service &io_service, uint16_t port)
      : server(io_service, endpoint(boost::asio::ip::udp::v4(), port)) {}

    stream make_stream() {
      return _dispatcher.make_stream();
    }

  private:

    detail::async_udp_server _server;

    detail::dispatcher _dispatcher;
  };

  server::server(boost::asio::io_service &io_service, endpoint ep)
    : _server(io_service, std::move(ep)) {
    _server.listen([this](auto session){ _dispatcher.register_session(session); });
  }

} // namespace strm

