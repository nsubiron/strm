#pragma once

#include "strm/detail/stream_state.h"
#include "strm/detail/token.h"
#include "strm/message.h"

#include <boost/asio/buffer.hpp>

#include <cassert>
#include <memory>

namespace strm {

namespace low_level {
  class server;
}

  class stream {
  public:

    stream() = delete;

    stream(const stream &) = default;
    stream(stream &&) = default;

    stream &operator=(const stream &) = default;
    stream &operator=(stream &&) = default;

    token_type get_token() const {
      return _shared_state->token();
    }

    template <typename ConstBufferSequence>
    bool write(ConstBufferSequence buffer) {
      return _shared_state->write(std::make_shared<strm::message>(buffer));
    }

    template <typename T>
    stream &operator<<(const T &rhs) {
      write(boost::asio::buffer(rhs));
      return *this;
    }

  private:

    friend class strm::low_level::server;

    stream(std::shared_ptr<detail::stream_state> state)
      : _shared_state(std::move(state)) {
      assert(_shared_state != nullptr);
    }

    std::shared_ptr<detail::stream_state> _shared_state;
  };

} // namespace strm
