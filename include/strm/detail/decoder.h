#pragma once

#include "strm/detail/udp_packet.h"
#include "strm/message.h"

#include <cassert>
#include <exception>
#include <functional>
#include <iostream>
#include <mutex>
#include <unordered_map>

namespace strm {
namespace detail {

  class message_holder {
  public:

    ~message_holder() {
      // if (_message != nullptr) {
      //   std::cerr << "deleting message holder holding a message "
      //             << _packets_written << "/" << _total_number_of_packets
      //             << "\n";
      // }
    }

    /// Return nullptr if the message is not yet ready.
    std::shared_ptr<message> write(const udp_packet &datagram) {
      std::lock_guard<std::mutex> guard(_mutex);

      _total_number_of_packets = datagram.total_number_of_packets();

      if (!_was_initialized) {
        assert(_message == nullptr);
        _message = std::make_shared<message>(datagram.message_size());
        _was_initialized = true;
      }

      if (_message == nullptr) {
        // ignore packet, this message was already processed.
        return nullptr;
      }

      unsigned char * const begin = _message->data();

      const auto i = datagram.packet_number();
      constexpr size_t packet_capacity = udp_packet::numeric_limits::packet_capacity();
      assert((i * packet_capacity) < datagram.message_size());
      void *dest = begin + (i * packet_capacity);
      std::memcpy(dest, datagram.data(), datagram.size());

      ++_packets_written;
      return _packets_written < datagram.total_number_of_packets() ? nullptr : std::move(_message);
    }

  private:

    std::mutex _mutex;

    bool _was_initialized = false; /// @todo

    size_t _total_number_of_packets = 0u; /// @todo

    size_t _packets_written = 0u;

    std::shared_ptr<message> _message;
  };

  template <typename Key, typename T>
  class object_map {
  public:

    ~object_map() {
      std::cerr << "deleting object_map with " << _map.size() << " objects\n";
    }

    std::shared_ptr<T> operator[](Key key) {
      std::lock_guard<std::mutex> guard(_mutex);
      auto it = _map.find(key);
      if (it == _map.end()) {
        auto result = _map.emplace(std::make_pair(key, std::make_shared<T>()));
        assert(result.second);
        it = result.first;
      }
      return it->second;
    }

    void erase(Key key) {
      std::lock_guard<std::mutex> guard(_mutex);
      _map.erase(key);
    }

  private:

    std::mutex _mutex;

    std::unordered_map<Key, std::shared_ptr<T>> _map;
  };

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
      auto message = _message_map[datagram.message_id()];
      assert(message != nullptr);
      auto result = message->write(datagram);
      if (result != nullptr) {
        _message_map.erase(datagram.message_id());
        _callback(std::move(result));
      }
    }

    size_t _payload;

    callback_type _callback;

    object_map<size_t, message_holder> _message_map;
  };

} // namespace detail
} // namespace strm
