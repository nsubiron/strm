#pragma once

#include "encoder.h"
#include "session.h"

#include <memory>
#include <mutex>

namespace strm {

  class stream_internal { /// @todo move-constructible only.
  public:

    explicit stream_internal(token_type token, std::shared_ptr<encoder> buffer_encoder)
      : _token(token),
        _encoder(std::move(buffer_encoder)),
        _session(nullptr) {}

    token_type token() const {
      return _token;
    }

    bool write(boost::asio::const_buffer buffer) {
      std::lock_guard<std::mutex> guard(_mutex);
      if (_session != nullptr) {
        for (auto &packet : _encoder->split_message(buffer)) {
          /// @todo temporary, we shouldn't create objects all the time.
          _session->enqueue_response(std::make_shared<udp_packet>(packet));
        }
        return true;
      }
      return false;
    }

    void set_session(shared_session session) {
      std::lock_guard<std::mutex> guard(_mutex);
      _session = std::move(session);
    }

  private:

    mutable std::mutex _mutex; /// @todo can be atomic.

    const token_type _token;

    std::shared_ptr<encoder> _encoder;

    shared_session _session;
  };

  class stream {
  public:

    explicit stream(std::shared_ptr<stream_internal> the_stream)
      : _the_stream(std::move(the_stream)) {}

    token_type get_token() const {
      assert(_the_stream != nullptr);
      return _the_stream->token();
    }

    bool write(boost::asio::const_buffer buffer) {
      assert(_the_stream != nullptr);
      return _the_stream->write(buffer); /// @todo should the synchronization be here?
    }

    template <typename T>
    stream &operator<<(T &&rhs) {
      write(boost::asio::buffer(std::forward<T>(rhs)));
      return *this;
    }

  private:

    std::shared_ptr<stream_internal> _the_stream;
  };

} // namespace strm
