#pragma once

#include "strm/low_level/server.h"
#include "strm/server.h"

namespace strm {
namespace low_level {

  class server_inspector {
  public:

    static size_t bytes_sent(const strm::low_level::server &srv) {
      return srv._server.get_bytes_sent();
    }

    static size_t bytes_sent(const strm::server &srv) {
      return bytes_sent(srv._server);
    }

    static size_t datagrams_sent(const strm::low_level::server &srv) {
      return srv._server.get_datagrams_sent();
    }

    static size_t datagrams_sent(const strm::server &srv) {
      return datagrams_sent(srv._server);
    }

    static size_t payload(const strm::low_level::server &) {
      return 512u;
    }

    static size_t payload(const strm::server &srv) {
      return payload(srv._server);
    }

    static size_t datagram_header_size(const strm::low_level::server &) {
      return 5u * 2u;
    }

    static size_t datagram_header_size(const strm::server &srv) {
      return datagram_header_size(srv._server);
    }

    static size_t datagram_capacity(const strm::low_level::server &srv) {
      return payload(srv) - datagram_header_size(srv);
    }

    static size_t datagram_capacity(const strm::server &srv) {
      return datagram_capacity(srv._server);
    }
  };

} // namespace low_level
} // namespace strm

