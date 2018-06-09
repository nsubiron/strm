#pragma once

#include "strm/detail/udp_packet.h"
#include "strm/message.h"

#include <cassert>
#include <exception>
#include <iostream>
#include <mutex>

namespace strm {
namespace detail {

  class decoder {
  public:

    using shared_message = std::shared_ptr<message>;
    using callback_type = std::function<void(shared_message)>;

    template <typename F>
    explicit decoder(size_t payload, F &&callback)
      : _payload(payload),
        _callback(std::forward<F>(callback)) {}

    void parse(const unsigned char *data, size_t size) {
      if (size != _payload) {
        std::cerr << "datagram discarded: invalid size " << size << "\n";
        return;
      }
      assert(data != nullptr);
      write_to_message(*reinterpret_cast<const udp_packet *>(data));
    }

  private:

    void write_to_message(const udp_packet &datagram) {
      std::lock_guard<std::mutex> guard(_mutex);
      if (_current_id != datagram.message_id()) {
        /// @todo should we have a better way?
        std::cout << "pushing message " << _percentage_ready << "% complete\n";
        _callback(std::move(_current)); /// @todo call async.
      }
      if (_current == nullptr) {
        _current = std::make_shared<message>(datagram.message_size());
        _current_id = datagram.message_id();
        _percentage_ready = 0.0f;
      }

      unsigned char * const begin = _current->data();

      const auto i = datagram.packet_number();
      constexpr size_t packet_capacity = udp_packet::numeric_limits::packet_capacity();
      assert((i * packet_capacity) < datagram.message_size());
      void *dest = begin + (i * packet_capacity);
      std::memcpy(dest, datagram.data(), datagram.size());

      _percentage_ready += 100.0f / static_cast<float>(datagram.total_number_of_packets());
    }

    std::mutex _mutex;

    size_t _payload;

    callback_type _callback;

    size_t _current_id = 0u;

    float _percentage_ready = 0.0f;

    shared_message _current;
  };

} // namespace detail
} // namespace strm
