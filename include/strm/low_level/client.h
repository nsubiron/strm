#pragma once

#include "strm/detail/decoder.h"
#include "strm/detail/strm_asio/udp_client.h"
#include "strm/detail/token.h"

namespace strm {
namespace low_level {

  class client {

    static auto get_endpoint(const token_type &token) {
      if (!token.endpoint.protocol_is_udp())
        throw std::invalid_argument("invalid token, only UDP tokens supported");
      return token.endpoint.to_udp_endpoint();
    }

  public:

    explicit client(boost::asio::io_service &io_service, const token_type &token)
      : _client(io_service, get_endpoint(token)) {
      _client.send_token(token.stream_id);
    }

    /// callback(std::shared_ptr<strm::message>);
    template <typename F>
    void listen(F &&callback) {
      auto decoder = std::make_shared<strm::detail::decoder>(
          512u,
          std::forward<F>(callback));
      _client.listen(512u, [=](const unsigned char *data, size_t size) {
        decoder->parse(data, size);
      });
    }

  private:

    strm::detail::strm_asio::udp_client _client;
  };

} // namespace low_level
} // namespace strm

