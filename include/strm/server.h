#pragma once

#include "async_udp_server.h"
#include "dispatcher.h"
#include "encoder.h"
#include "session.h"
#include "stream.h"

#include <memory>
#include <mutex>

namespace strm {

  class server {
  public:

    using endpoint = async_udp_server::endpoint;

    explicit server(boost::asio::io_service &io_service, endpoint ep);

    explicit server(boost::asio::io_service &io_service, uint16_t port)
      : server(io_service, endpoint(boost::asio::ip::udp::v4(), port)) {}

    stream make_stream() {
      return _dispatcher.make_stream(_encoder);
    }

  private:

    async_udp_server _server;

    dispatcher _dispatcher;

    std::shared_ptr<encoder> _encoder;
  };

  server::server(boost::asio::io_service &io_service, endpoint ep)
    : _server(io_service, std::move(ep)),
      _encoder(std::make_shared<encoder>()) {
    _server.listen([this](auto active_session){
      _dispatcher.register_session(std::move(active_session));
    });
  }

} // namespace strm

