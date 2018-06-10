#pragma once

#include "strm/detail/encoder.h"
#include "strm/detail/profiler.h"
#include "strm/detail/session.h"
#include "strm/detail/token.h"
#include "strm/message.h"

#include <memory>
#include <mutex>

namespace strm {
namespace detail {

  /// Handles the synchronization of the shared session.
  class session_holder {
  public:

    void set_session(shared_session session) {
      std::lock_guard<std::mutex> guard(_mutex);
      _session = std::move(session);
    }

  protected:

    shared_session get_session() const {
      std::lock_guard<std::mutex> guard(_mutex);
      return _session;
    }

  private:

    mutable std::mutex _mutex; /// @todo it can be atomic

    shared_session _session;
  };

  /// Shared state among all the copies of a stream. Provides access to the
  /// underlying UDP session if active.
  class stream_state : public session_holder {
  public:

    explicit stream_state(const token_type &token) : _token(token) {}

    stream_state(const stream_state &) = delete;
    stream_state &operator=(const stream_state &) = delete;

    const token_type &token() const {
      return _token;
    }

    bool write(std::shared_ptr<const message> data) {
      /// @todo this function can already be async.
      auto session = get_session();
      if (session == nullptr) {
        return false;
      }
      {
        STRM_PROFILE_SCOPE(server, encode_and_enqueue);
        for (auto &packet : _encoder.split_message(data->buffer())) {
          session->enqueue_response(std::make_shared<udp_packet>(packet));
        }
      }
      return true;
    }

  private:

    const token_type _token;

    encoder _encoder;
  };

} // namespace detail
} // namespace strm
