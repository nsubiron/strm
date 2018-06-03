#pragma once

#include "session.h"
#include "stream.h"

#include <exception>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace strm {

  class dispatcher {
  public:

    stream make_stream(std::shared_ptr<encoder> buffer_encoder) {
      std::lock_guard<std::mutex> guard(_mutex);
      auto token = ++_next_token; // token zero never happens (unless overflow...).
      auto ptr = std::make_shared<stream_internal>(token, std::move(buffer_encoder));
      auto result = _stream_map.emplace(std::make_pair(token, ptr));
      if (!result.second)
        throw std::runtime_error("failed to create stream!");
      return stream{ptr};
    }

    void register_session(shared_session session) {
      assert(session != nullptr);
      std::lock_guard<std::mutex> guard(_mutex);
      auto search = _stream_map.find(session->token());
      if (search != _stream_map.end()) {
        assert(search->second != nullptr);
        search->second->set_session(std::move(session));
      } else {
        std::cerr << "Invalid session token " << session->token()
                  << ", no stream available with that token\n";
      }
    }

  private:

    // We use a mutex here, but we assume that sessions and streams won't be
    // created too often.
    std::mutex _mutex;

    token_type _next_token = 0u;

    std::unordered_map<token_type, std::shared_ptr<stream_internal>> _stream_map;
  };

} // namespace strm
