#pragma once

#include "strm/detail/thread_group.h"
#include "strm/detail/token.h"
#include "strm/low_level/client.h"

#include <boost/asio/io_service.hpp>

namespace strm {

  class client {
  public:

    explicit client(const token_type &token)
      : _client(_io_service, token) {}

    template <typename F>
    void listen(F &&callback) {
      _client.listen(std::forward<F>(callback));
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

    strm::low_level::client _client;

    strm::detail::thread_group _workers;
  };

} // namespace strm

