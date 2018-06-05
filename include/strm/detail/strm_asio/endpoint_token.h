#pragma once

#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

#include <cstdint>
#include <cassert>

namespace strm {
namespace detail {
namespace strm_asio {

  /// Serialization of a boost endpoint, contains no dynamic memory.
  class endpoint_token {
  private:

    uint16_t _port;

    enum class protocol : uint8_t {
      not_set,
      tcp,
      udp
    } _protocol = protocol::not_set;

    enum class address : uint8_t {
      not_set,
      ip_v4,
      ip_v6
    } _address_type = address::not_set;

    union {
      boost::asio::ip::address_v4::bytes_type v4;
      boost::asio::ip::address_v6::bytes_type v6;
    } _address;

  private:

    template <typename P>
    static constexpr protocol get_protocol() {
      return std::is_same<P, boost::asio::ip::tcp>::value ? protocol::tcp : protocol::udp;
    }

    void set_address(const boost::asio::ip::address &addr) {
      if (addr.is_v4()) {
        _address_type = address::ip_v4;
        _address.v4 = addr.to_v4().to_bytes();
      } else if (addr.is_v6()) {
        _address_type = address::ip_v6;
        _address.v6 = addr.to_v6().to_bytes();
      } else {
        throw std::invalid_argument("invalid ip address!");
      }
    }

    boost::asio::ip::address get_address() const {
      if (_address_type == address::ip_v4)
        return boost::asio::ip::address_v4(_address.v4);
      return boost::asio::ip::address_v6(_address.v6);
    }

    template <typename P>
    boost::asio::ip::basic_endpoint<P> get_endpoint() const {
      assert(is_valid());
      assert(get_protocol<P>() == _protocol);
      return {get_address(), _port};
    }

  public:

    endpoint_token() = default;
    endpoint_token(const endpoint_token &) = default;

    template <typename P>
    endpoint_token(const boost::asio::ip::basic_endpoint<P> &ep)
      : _port(ep.port()),
        _protocol(get_protocol<P>()) {
      set_address(ep.address());
    }

    bool is_valid() const {
      return ((_protocol != protocol::not_set) && (_address_type != address::not_set));
    }

    bool address_is_v4() const {
      return _address_type == address::ip_v4;
    }

    bool address_is_v6() const {
      return _address_type == address::ip_v6;
    }

    bool protocol_is_udp() const {
      return _protocol == protocol::udp;
    }

    bool protocol_is_tcp() const {
      return _protocol == protocol::tcp;
    }

    boost::asio::ip::udp::endpoint to_udp_endpoint() const {
      return get_endpoint<boost::asio::ip::udp>();
    }

    boost::asio::ip::tcp::endpoint to_tcp_endpoint() const {
      return get_endpoint<boost::asio::ip::tcp>();
    }
  };

} // namespace strm_asio
} // namespace detail
} // namespace strm
