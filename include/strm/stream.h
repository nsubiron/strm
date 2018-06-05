#pragma once

#include "detail/stream_state.h"
#include "detail/token.h"

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

    bool write(boost::asio::const_buffer buffer) {
      return _shared_state->write(buffer);
    }

    template <typename T>
    stream &operator<<(T &&rhs) {
      write(boost::asio::buffer(std::forward<T>(rhs)));
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
