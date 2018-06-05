#pragma once

#include "strm/detail/session.h"
#include "strm/detail/stream_state.h"

#include <exception>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace strm {
namespace detail {

  class dispatcher {
  public:

    explicit dispatcher(strm_asio::endpoint_token ep)
      : _cached_token{0u, ep} {}

    std::shared_ptr<stream_state> make_stream() {
      std::lock_guard<std::mutex> guard(_mutex);
      ++_cached_token.stream_id; // id zero only happens in overflow.
      auto ptr = std::make_shared<stream_state>(_cached_token);
      auto result = _stream_map.emplace(std::make_pair(_cached_token.stream_id, ptr));
      if (!result.second)
        throw std::runtime_error("failed to create stream!");
      return ptr;
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
                  << ", no stream available with that id\n";
      }
    }

  private:

    // We use a mutex here, but we assume that sessions and streams won't be
    // created too often.
    std::mutex _mutex;

    token_type _cached_token;

    std::unordered_map<
        strm::token_type::stream_id_type,
        std::shared_ptr<stream_state>> _stream_map;
  };

} // namespace detail
} // namespace strm
