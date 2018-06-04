#include "message.h"

#include <climits>
#include <random>

namespace util {

  message message::make_empty(size_t size) {
    return size == 0u ?
        message() :
        message(size, std::make_unique<message::value_type[]>(size));
  }

  message message::make_random(size_t size) {
    using random_bytes_engine = std::independent_bits_engine<
      std::default_random_engine,
      CHAR_BIT,
      unsigned char>;
    random_bytes_engine rbe;
    auto message = make_empty(size);
    std::generate(message.begin(), message.end(), std::ref(rbe));
    return message;
  }

  message message::make_from_buffer(boost::asio::const_buffer buffer) {
    auto copy = make_empty(buffer.size());
    std::memcpy(copy.data(), buffer.data(), buffer.size());
    return copy;
  }

  std::string message::to_hex_string(size_t length) const {
    length = std::min(size(), length);
    auto buffer = std::make_unique<char[]>(2u * length + 1u);
    for (auto i = 0u; i < length; ++i)
      sprintf(&buffer[2u * i], "%02x", data()[i]);
    if (length < size())
      return std::string(buffer.get()) + std::string("...");
    return std::string(buffer.get());
  }

} // namespace util
