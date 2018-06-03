#include "message.h"

#include <climits>
#include <random>

namespace message {

  std::string Message::to_hex_string() const {
    const size_t length = std::min(size(), 16ul);
    auto buffer = std::make_unique<char[]>(2u * length + 1u);
    for (auto i = 0u; i < length; ++i)
      sprintf(&buffer[2u * i], "%02x", data()[i]);
    if (length < size())
      return std::string(buffer.get()) + std::string("...");
    return std::string(buffer.get());
  }

  Message make_random(size_t size) {
    using random_bytes_engine = std::independent_bits_engine<
      std::default_random_engine,
      CHAR_BIT,
      unsigned char>;
    random_bytes_engine rbe;
    auto message = make_empty(size);
    std::generate(message.begin(), message.end(), std::ref(rbe));
    return message;
  }

} // namespace message
