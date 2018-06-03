#pragma once

#include <boost/asio/buffer.hpp>

#include <algorithm>
#include <memory>
#include <ostream>
#include <string>

namespace message {

  /// A message containing a buffer with raw data.
  class Message {
  public:

    using value_type = unsigned char;
    using iterator = value_type *;
    using const_iterator = const value_type *;

    Message() = default;

    explicit Message(std::unique_ptr<value_type[]> data, size_t size)
      : _size(size),
        _data(std::move(data)) {}

    size_t size() const {
      return _size;
    }

    const value_type *data() const {
      return _data.get();
    }

    value_type *data() {
      return _data.get();
    }

    boost::asio::const_buffer buffer() const {
      return {data(), size()};
    }

    boost::asio::mutable_buffer buffer() {
      return {data(), size()};
    }

    iterator begin() {
      return data();
    }

    const_iterator begin() const {
      return data();
    }

    iterator end() {
      return data() + size();
    }

    const_iterator end() const {
      return data() + size();
    }

    bool operator==(const Message &rhs) const {
      return std::equal(begin(), end(), rhs.begin());
    }

    bool operator!=(const Message &rhs) const {
      return !(*this == rhs);
    }

    std::string to_hex_string() const;

    std::ostream &print(std::ostream &out) const {
      out << "[" << size() << " bytes] " << to_hex_string();
      return out;
    }

  private:

    size_t _size = 0u;

    std::unique_ptr<value_type[]> _data = nullptr;
  };

  static inline std::ostream &operator<<(std::ostream &out, const Message &msg) {
    return msg.print(out);
  }

  static inline Message make_empty(size_t size = 0u) {
    return size == 0u ?
        Message() :
        Message(std::make_unique<Message::value_type[]>(size), size);
  }

  Message make_random(size_t size);

} // namespace message
