#pragma once

#include "udp_packet_iterator.h"

#include <atomic>
#include <exception>

namespace strm {

  class encoder {
  public:

    auto split_message(boost::asio::const_buffer buffer) {
      if (buffer.size() > udp_packet::numeric_limits::maximum_message_size())
        throw std::invalid_argument("message size too big!");
      return detail::udp_packet_const_iterator(_next_message_id++, buffer);
    }

  private:

    std::atomic<udp_packet::size_type> _next_message_id{0u};
  };

} // namespace strm
