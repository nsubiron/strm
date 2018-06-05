#pragma once

#include "strm/detail/strm_asio/endpoint_token.h"

namespace strm {
namespace detail {

  class dispatcher;

} // namespace detail

  /// Contains the necessary information for a client to subscribe to a stream.
  /// The token can be treated as a plain data, contains no dynamic memory.
  struct token_type {
  public:

    token_type() = default;
    token_type(const token_type &) = default;

  private:

    friend class detail::dispatcher;

    using stream_id_type = uint32_t;

    token_type(stream_id_type id, detail::strm_asio::endpoint_token ep)
      : stream_id(id),
        endpoint(ep) {}

    stream_id_type stream_id = 0u;

    detail::strm_asio::endpoint_token endpoint;
  };

} // namespace strm
