#pragma once

#include <boost/asio/buffer.hpp>

#include <cassert>
#include <memory>

namespace strm {
namespace detail {

  /// A message owns a buffer with raw data.
  class message : public std::enable_shared_from_this<message> {

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
  public:

    message() = default;

    message(const message &) = delete;
    message &operator=(const message &) = delete;

    template <typename ConstBufferSequence>
    explicit message(ConstBufferSequence source)
      : _size(boost::asio::buffer_size(source)),
        _data(std::make_unique<value_type[]>(_size)) {
#ifndef NDEBUG
      auto bytes_copied =
#endif // NDEBUG
      boost::asio::buffer_copy(buffer(), source);
#ifndef NDEBUG
      assert(bytes_copied == size());
#endif // NDEBUG
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

  private:

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

  private:

    boost::asio::mutable_buffer buffer() {
      return {data(), size()};
    }

    // =========================================================================
    // -- Private members ------------------------------------------------------
    // =========================================================================
  private:

    size_t _size = 0u;

    std::unique_ptr<value_type[]> _data = nullptr;
  };

} // namespace detail
} // namespace strm
