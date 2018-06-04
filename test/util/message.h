#pragma once

#include <boost/asio/buffer.hpp>

#include <algorithm>
#include <memory>
#include <ostream>
#include <string>

namespace util {

  /// A message containing a buffer with raw data.
  class message {

    // =========================================================================
    // -- Static generators ----------------------------------------------------
    // =========================================================================
  public:

    static message make_empty(size_t size);

    static message make_random(size_t size);

    static message make_from_buffer(boost::asio::const_buffer buffer);

    // =========================================================================
    // -- Typedefs -------------------------------------------------------------
    // =========================================================================
  public:

    using value_type = unsigned char;
    using iterator = value_type *;
    using const_iterator = const value_type *;

    // =========================================================================
    // -- Construction and assignment ------------------------------------------
    // =========================================================================
  private:

    explicit message(size_t size, std::unique_ptr<value_type[]> data)
      : _size(size),
        _data(std::move(data)) {}

  public:

    message() = default;

    message(const message &) = delete;
    message &operator=(const message &) = delete;

    message(message &&rhs) : message(rhs._size, std::move(rhs._data)) {
      rhs._size = 0u;
    }

    message &operator=(message &&rhs) {
      _size = rhs._size;
      _data = std::move(rhs._data);
      rhs._size = 0u;
      return *this;
    }

    // =========================================================================
    // -- Data access ----------------------------------------------------------
    // =========================================================================
  public:

    size_t size() const {
      return _size;
    }

    const value_type *data() const {
      return _data.get();
    }

    value_type *data() {
      return _data.get();
    }

    // =========================================================================
    // -- Conversions to boost::asio::buffer -----------------------------------
    // =========================================================================
  public:

    boost::asio::const_buffer buffer() const {
      return {data(), size()};
    }

    boost::asio::mutable_buffer buffer() {
      return {data(), size()};
    }

    operator boost::asio::const_buffer() const {
      return buffer();
    }

    // =========================================================================
    // -- Iteration support ----------------------------------------------------
    // =========================================================================
  public:

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

    // =========================================================================
    // -- Comparison -----------------------------------------------------------
    // =========================================================================
  public:

    bool operator==(const message &rhs) const {
      return std::equal(begin(), end(), rhs.begin());
    }

    bool operator!=(const message &rhs) const {
      return !(*this == rhs);
    }

    // =========================================================================
    // -- Printing -------------------------------------------------------------
    // =========================================================================
  public:

    std::string to_hex_string(size_t length = 16u) const;

    std::ostream &print(std::ostream &out) const {
      out << "[" << size() << " bytes] " << to_hex_string();
      return out;
    }

    // =========================================================================
    // -- Private members ------------------------------------------------------
    // =========================================================================
  private:

    size_t _size = 0u;

    std::unique_ptr<value_type[]> _data = nullptr;
  };

  static inline std::ostream &operator<<(std::ostream &out, const message &msg) {
    return msg.print(out);
  }

} // namespace util
