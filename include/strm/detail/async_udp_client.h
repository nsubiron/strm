#pragma once

#include "token.h"
#include "udp_packet.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>

#include <array>

namespace strm {
namespace detail {

  /// Not yet async, barely a client.
  class async_udp_client {
  public:

    using endpoint = boost::asio::ip::udp::endpoint;

    explicit async_udp_client(boost::asio::io_service &io_service, endpoint ep)
      : _endpoint(std::move(ep)),
        _socket(io_service) {
      _socket.open(_endpoint.protocol());
    }

    void subscribe_to_stream(token_type token) {
      _socket.send_to(
          boost::asio::buffer(&token, sizeof(token)),
          _endpoint);
    }

    bool read_packet(udp_packet &packet) {
      std::array<unsigned char, sizeof(udp_packet)> buffer;
      const auto length = _socket.receive_from(boost::asio::buffer(buffer), _endpoint);
      const bool success = length == sizeof(udp_packet);
      if (success) {
        std::memcpy(&packet, buffer.data(), length);
      }
      return success;
    }

  private:

    endpoint _endpoint;

    boost::asio::ip::udp::socket _socket;
  };

} // namespace detail
} // namespace strm

