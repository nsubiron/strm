#pragma once

#include "strm/detail/token.h"
#include "strm/detail/udp_packet.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/io_service_strand.hpp>
#include <boost/asio/ip/udp.hpp>

#include <iostream>

namespace strm {
namespace detail {
namespace strm_asio {

  class udp_client {
  public:

    using endpoint = boost::asio::ip::udp::endpoint;
    using error_code = boost::system::error_code;

    explicit udp_client(boost::asio::io_service &io_service, endpoint ep)
      : _endpoint(std::move(ep)),
        _socket(io_service),
        _strand(io_service) {
      _socket.open(_endpoint.protocol());
    }

    /// Not async, throws on failure.
    void send_token(uint32_t token) {
#ifndef NDEBUG
      auto bytes_sent =
#endif // NDEBUG
      _socket.send_to(boost::asio::buffer(&token, sizeof(token)), _endpoint);
#ifndef NDEBUG
      assert(bytes_sent == sizeof(token));
#endif // NDEBUG
    }

    // callback(const unsigned char *, size_t)
    template <typename F>
    void listen(size_t bytes, F callback) {
      post([=](){ do_read(bytes, callback); });
    }

  private:

    template <typename F>
    void post(F &&job) {
      _socket.get_io_service().post(std::forward<F>(job));
    }

    template <typename F>
    void do_read(size_t bytes, F callback) {
      using raw_data_t = unsigned char;
      auto datagram = std::shared_ptr<raw_data_t>(
          new raw_data_t[bytes],
          std::default_delete<raw_data_t[]>());

      auto handle_datagram = [=](const error_code &ec, size_t bytes_readed) {
        if (!ec || ec == boost::asio::error::message_size) {
          callback(const_cast<const raw_data_t *>(datagram.get()), bytes_readed);
        } else {
          std::cerr << "Error reading datagram from " << _endpoint
                    << ": " << ec.message() << "\n";
        }
      };

      auto handle_receive = [=](const error_code &ec, size_t bytes_transferred) {
        post([=](){ handle_datagram(ec, bytes_transferred); });
        do_read(bytes, callback);
      };

      _socket.async_receive_from(
          boost::asio::buffer(datagram.get(), bytes),
          _endpoint,
          _strand.wrap(handle_receive));
    }

    endpoint _endpoint;

    boost::asio::ip::udp::socket _socket;

    boost::asio::io_service::strand _strand;
  };

} // namespace strm_asio
} // namespace detail
} // namespace strm

