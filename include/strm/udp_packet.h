#pragma once

#include <boost/asio/buffer.hpp>

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>

namespace strm {

  /// Many udp_packets compose a message.
  class udp_packet {
  public:

    using size_type = uint16_t;

  private:

    enum Fields : size_type {
      MESSAGE_ID,
      TOTAL_NUMBER_OF_PACKETS,
      REMAINDER,
      PACKET_NUMBER,
      SIZE,
      DATA
    };

    static constexpr size_type total_number_of_units = 256u;
    static constexpr size_type header_units = DATA;
    static_assert(total_number_of_units > header_units, "Packet size too small!");
    static constexpr size_type message_units = total_number_of_units - header_units;
    static constexpr size_type message_size_in_bytes = message_units * sizeof(size_type);

  public:

    struct numeric_limits {

      static constexpr size_t maximum_number_of_packets() {
        return std::numeric_limits<size_type>::max();
      }

      static constexpr size_t packet_capacity() {
        return message_size_in_bytes;
      }

      static constexpr size_t maximum_message_size() {
        return packet_capacity() * maximum_number_of_packets();
      }

    };

    size_t size() const {
      return _data[SIZE];
    }

    const void *data() const {
      return &_data[DATA];
    }

    size_t message_id() const {
      return _data[MESSAGE_ID];
    }

    size_t message_size() const {
      return numeric_limits::packet_capacity() * (total_number_of_packets() - 1u) + _data[REMAINDER];
    }

    size_t packet_number() const {
      return _data[PACKET_NUMBER];
    }

    void set_packet_number(size_t packet_number) {
      assert(packet_number < std::numeric_limits<size_type>::max());
      _data[PACKET_NUMBER] = packet_number;
    }

    size_t total_number_of_packets() const {
      return _data[TOTAL_NUMBER_OF_PACKETS];
    }

    void reset(size_t message_id, size_t total_number_of_packets, size_t remainder) {
      assert(message_id < std::numeric_limits<size_type>::max());
      assert(total_number_of_packets < std::numeric_limits<size_type>::max());
      assert(remainder < std::numeric_limits<size_type>::max());
      assert(total_number_of_packets > 0u);

      _data[MESSAGE_ID] = static_cast<size_type>(message_id);
      _data[TOTAL_NUMBER_OF_PACKETS] = static_cast<size_type>(total_number_of_packets);
      _data[REMAINDER] = static_cast<size_type>(remainder);
      _data[SIZE] = 0u;
    }

    void write(const void *buffer, size_t length) {
      assert(length <= message_size_in_bytes);
      std::memcpy(&_data[DATA], buffer, length);
      _data[SIZE] = length;
    }

    boost::asio::const_buffer buffer() const {
      return {reinterpret_cast<const void *>(this), sizeof(*this)};
    }

  private:

    std::array<size_type, total_number_of_units> _data;
  };

} // namespace strm

static_assert(sizeof(::strm::udp_packet) == 512u, "Invalid udp_packet size!");
static_assert(std::is_trivial<::strm::udp_packet>::value, "Try to keep udp_packet a trivial type");
