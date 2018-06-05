#pragma once

#include "strm/detail/dispatcher.h"
#include "strm/detail/strm_asio/udp_server.h"
#include "strm/stream.h"

namespace strm {
namespace low_level {

  class server {
    using underlying_server = strm::detail::strm_asio::udp_server;
  public:

    using endpoint = underlying_server::endpoint;

    explicit server(boost::asio::io_service &io_service, const endpoint &ep)
      : _server(io_service, ep),
        _dispatcher(ep) {
      _server.listen([this](auto session){
        _dispatcher.register_session(session);
      });
    }

    explicit server(boost::asio::io_service &io_service, uint16_t port)
      : server(io_service, endpoint(boost::asio::ip::udp::v4(), port)) {}

    explicit server(boost::asio::io_service &io_service, const std::string &address, uint16_t port)
      : server(io_service, endpoint(boost::asio::ip::address::from_string(address), port)) {}

    stream make_stream() {
      return _dispatcher.make_stream();
    }

  private:

    underlying_server _server;

    strm::detail::dispatcher _dispatcher;
  };

} // namespace low_level
} // namespace strm

