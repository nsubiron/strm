#pragma once

#include <strm/message.h>

#include <algorithm>
#include <memory>
#include <ostream>
#include <string>

namespace util {
namespace message {

  using shared_message = std::shared_ptr<strm::message>;

  static inline shared_message make_empty() {
    return std::make_shared<strm::message>();
  }

  shared_message make_empty(size_t size);

  shared_message make_random(size_t size);

  template <typename T>
  static inline shared_message make(const T &buffer) {
    return std::make_shared<strm::message>(boost::asio::buffer(buffer));
  }

  static inline std::string as_string(const strm::message &msg) {
    return {reinterpret_cast<const char *>(msg.data()), msg.size()};
  }

  std::string to_hex_string(const strm::message &msg, size_t length = 16u);

} // namespace message
} // namespace util

namespace strm {

  static inline unsigned char *begin(message &msg) {
    return msg.data();
  }

  static inline const unsigned char *begin(const message &msg) {
    return msg.data();
  }

  static inline unsigned char *end(message &msg) {
    return msg.data() + msg.size();
  }

  static inline const unsigned char *end(const message &msg) {
    return msg.data() + msg.size();
  }

  static inline std::ostream &operator<<(std::ostream &out, const message &msg) {
    out << "[" << msg.size() << " bytes] " << util::message::to_hex_string(msg);
    return out;
  }

  static inline bool operator==(const message &lhs, const message &rhs) {
    return
        (lhs.size() == rhs.size()) &&
        std::equal(begin(lhs), end(lhs), begin(rhs));
  }

  static inline bool operator!=(const message &lhs, const message &rhs) {
    return !(lhs == rhs);
  }

} // namespace strm
