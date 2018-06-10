#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/asio/io_service_strand.hpp>
#include <boost/asio/ip/udp.hpp>

#include <atomic>
#include <iostream> /// @todo
#include <memory>

namespace strm {
namespace detail {
namespace strm_asio {

  class udp_session;

  /// ==========================================================================
  /// -- udp_server ------------------------------------------------------------
  /// ==========================================================================

  /// An asynchronous UDP stream server.
  ///
  /// Call `listen` to start listening for queries at the given endpoint. On
  /// each query (which is expected to consist on a single message of type
  /// `uint32_t`), a new `udp_session` is activated and passed to the call-back
  /// function provided in `listen`. The session object provides an
  /// `enqueue_response` function for writing data to the socket.
  ///
  /// @warning sessions should not outlive the server.
  class udp_server {
  public:

    using endpoint = boost::asio::ip::udp::endpoint;
    using error_code = boost::system::error_code;
    using shared_session = std::shared_ptr<udp_session>; /// @todo make const

    udp_server(boost::asio::io_service &io_service, endpoint ep)
      : _socket(io_service, std::move(ep)),
        _strand(io_service) {}

    /// Start listening for queries.
    ///
    /// On each query the @a callback function is called; @callback should
    /// accept a @a shared_session as single argument, its return value is
    /// ignored.
    template <typename F>
    void listen(F callback) {
      post([=](){ open_session(callback); });
    }

    size_t get_bytes_sent() const {
      return _bytes_sent;
    }

    size_t get_datagrams_sent() const {
      return _datagrams_sent;
    }

  private:

    friend udp_session;

    template <typename F>
    void post(F &&job) {
      _socket.get_io_service().post(std::forward<F>(job));
    }

    template <typename F>
    void open_session(F callback);

    /// See documentation at udp_session::enqueue_response.
    template <typename T>
    void enqueue_response(shared_session session, std::shared_ptr<T> data);

    boost::asio::ip::udp::socket _socket;

    boost::asio::io_service::strand _strand;

    std::atomic_size_t _bytes_sent{0u};

    std::atomic_size_t _datagrams_sent{0u};
  };

  /// ==========================================================================
  /// -- udp_session -----------------------------------------------------------
  /// ==========================================================================

  /// A session that represents a stream of data towards the client. Each time a
  /// client sends a "token" query, a new session is activated.
  class udp_session : public std::enable_shared_from_this<udp_session> {
  public:

    explicit udp_session(udp_server &parent) : _parent(parent) {}

    /// Retrieve current session's token.
    uint32_t token() const {
      return _token;
    }

    /// Enqueues some data to be sent through the network; @a data is required
    /// to have a member function `buffer()` returning something convertible by
    /// `boost::asio::buffer()`.
    template <typename T>
    void enqueue_response(std::shared_ptr<T> data) {
      _parent.enqueue_response(shared_from_this(), std::move(data));
    }

  private:

    friend class udp_server;

    udp_server &_parent;

    udp_server::endpoint _remote_endpoint;

    uint32_t _token;
  };

  /// ==========================================================================
  /// -- udp_server implementations --------------------------------------------
  /// ==========================================================================

  template <typename F>
  void udp_server::open_session(F callback) {
    auto session = std::make_shared<udp_session>(*this);

    // This is call for each new query, calls the callback function.
    auto handle_query = [=](const error_code &ec, size_t){
      if (!ec || ec == boost::asio::error::message_size) {
        callback(session);
      } else {
        std::cerr << "Error handling query from " << session->_remote_endpoint
                  << ": " << ec.message() << "\n";
      }
    };

    // Called after receiving a message. post a handle query and open another
    // session immediately.
    auto handle_receive = [=](const error_code &ec, size_t bytes_transferred) {
      post([=](){ handle_query(ec, bytes_transferred); });
      open_session(callback);
    };

    _socket.async_receive_from(
        boost::asio::buffer(&session->_token, sizeof(session->_token)),
        session->_remote_endpoint,
        _strand.wrap(handle_receive));
  }

  template <typename T>
  void udp_server::enqueue_response(shared_session session, std::shared_ptr<T> data) {

    // Explicitly capturing both objects we ensure they live as long as this
    // lambda. The standard guarantees that explicit captures are not optimized
    // away.
    auto handle_sent = [this, session, data](const error_code &ec, size_t bytes_sent) {
      if (ec) {
        std::cerr << "Error sending response to " << session->_remote_endpoint
                  << ": " << ec.message() << "\n";
      } else {
        _bytes_sent += bytes_sent;
        ++_datagrams_sent;
      }
    };

    _socket.async_send_to(
        boost::asio::buffer(data->buffer()),
        session->_remote_endpoint,
        _strand.wrap(handle_sent));
  }

} // namespace strm_asio
} // namespace detail
} // namespace strm
