#pragma once

#include "strm/detail/udp_packet.h"

#include <boost/asio/buffer.hpp>

#include <ostream>

namespace strm {
namespace detail {

  class udp_packet_end_iterator {};

  /// Splits a message into one or more udp_packets.
  class udp_packet_const_iterator {
  public:

    using iterator = udp_packet_const_iterator;
    using value_type = udp_packet;
    using reference = const udp_packet&;
    using pointer = const udp_packet*;

    explicit udp_packet_const_iterator(size_t message_id, boost::asio::const_buffer buffer)
      : _buffer(buffer) {
      constexpr size_t packet_capacity = udp_packet::numeric_limits::packet_capacity();
      const auto length = _buffer.size();

      // Ceil to the nearest integer. If length is zero, number_of_packets = 1.
      const size_t number_of_packets = 1u + ((length - 1u) / packet_capacity);
      assert(number_of_packets * udp_packet::numeric_limits::packet_capacity() >= length);
      const size_t remainder = length - (number_of_packets - 1u) * packet_capacity;

      _packet.reset(message_id, number_of_packets, remainder);
      increment(0u);
    }

    static constexpr size_t packet_size() {
      return sizeof(udp_packet);
    }

    size_t number_of_packets() const {
      return _packet.total_number_of_packets();
    }

    bool at_end() const {
      return _packet.packet_number() == _packet.total_number_of_packets();
    }

    reference operator*() const {
      assert(!at_end());
      return _packet;
    }

    pointer operator->() const {
      assert(!at_end());
      return &_packet;
    }

    iterator &operator++() { // ++i
      increment(_packet.packet_number() + 1);
      return *this;
    }

    bool operator!=(const udp_packet_end_iterator &) const {
      return !at_end();
    }

    std::ostream &print(std::ostream &out) const {
      out << _packet.message_size() << " bytes ("
          << _packet.total_number_of_packets() << " packets of "
          << packet_size() << " bytes)";
      return out;
    }

  private:

    void increment(const size_t packet_number) {
      _packet.set_packet_number(packet_number);
      if (at_end())
        return;
      constexpr size_t packet_capacity = udp_packet::numeric_limits::packet_capacity();
      const auto written_size = packet_number * packet_capacity;
      assert(written_size < _buffer.size());
      const auto size_left = _buffer.size() - written_size;
      auto begin = reinterpret_cast<const char *>(_buffer.data());
      const auto this_packet_size = std::min(packet_capacity, size_left);
      _packet.write(begin + written_size, this_packet_size);
    }

    boost::asio::const_buffer _buffer;

    udp_packet _packet;
  };

  static inline std::ostream &operator<<(std::ostream &out, const udp_packet_const_iterator &it) {
    return it.print(out);
  }

  // Static functions that allow range-based for-loop iterations.

  static inline udp_packet_const_iterator begin(udp_packet_const_iterator &splitter) {
    return splitter;
  }

  static inline udp_packet_end_iterator end(udp_packet_const_iterator &) {
    return udp_packet_end_iterator();
  }

} // namespace detail
} // namespace strm
