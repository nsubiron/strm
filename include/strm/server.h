#pragma once

#include "strm/detail/thread_group.h"
#include "strm/low_level/server.h"

#include <boost/asio/io_service.hpp>

#include <cstdint>
#include <string>

namespace strm {

  class server {
  public:

    explicit server(uint16_t port)
      : _server(_io_service, port) {}

    explicit server(const std::string &address, uint16_t port)
      : _server(_io_service, address, port) {}

    stream make_stream() {
      return _server.make_stream();
    }

    void run() {
      _io_service.run();
    }

    void async_run(std::size_t worker_threads) {
      _workers.create_threads(worker_threads, [this](){ run(); });
    }

    void stop() {
      _io_service.stop();
      _workers.join_all();
    }

  private:

    boost::asio::io_service _io_service;

    strm::low_level::server _server;

    strm::detail::thread_group _workers;
  };

} // namespace strm

