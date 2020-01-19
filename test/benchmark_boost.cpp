// These two need to be the first includes.
#include "util/profiler.h"
#include <gtest/gtest.h>

#include <strm/detail/thread_group.h>

#include "util/message.h"
#include "util/sync_ostream.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio.hpp>

using namespace std::chrono_literals;
using namespace util::message;

////////////////////////////////////////////////////////////////////////////////

  // struct datagram {
  //   std::array<uint16_t, 2u> header; // count, max.
  //   boost::asio::const_buffer data;
  //   std::shared_ptr<T> parent;

  //   auto buffer() const {
  //     return std::array<boost::asio::const_buffer, 2u>{
  //       boost::asio::buffer(header),
  //       data
  //     }
  //   }
  // };

  // class udp_srv {
  // public:

  //   using endpoint = boost::asio::ip::udp::endpoint;
  //   using error_code = boost::system::error_code;
  //   using shared_session = std::shared_ptr<udp_session>; /// @todo make const

  //   udp_srv(boost::asio::io_service &io_service, endpoint ep)
  //     : _socket(io_service, std::move(ep)),
  //       _strand(io_service) {}

  //   template <typename F>
  //   void listen(F callback) {
  //     post([=](){ open_session(callback); });
  //   }

  // private:

  //   friend udp_session;

  //   template <typename F>
  //   void post(F &&job) {
  //     _socket.get_io_service().post(std::forward<F>(job));
  //   }

  //   template <typename F>
  //   void open_session(F callback) {
  //     auto session = std::make_shared<udp_session>(*this);

  //     // This is call for each new query, calls the callback function.
  //     auto handle_query = [=](const error_code &ec, size_t){
  //       if (!ec || ec == boost::asio::error::message_size) {
  //         callback(session);
  //       } else {
  //         std::cerr << "Error handling query from " << session->_remote_endpoint
  //                   << ": " << ec.message() << "\n";
  //       }
  //     };

  //     // Called after receiving a message. post a handle query and open another
  //     // session immediately.
  //     auto handle_receive = [=](const error_code &ec, size_t bytes_transferred) {
  //       post([=](){ handle_query(ec, bytes_transferred); });
  //       open_session(callback);
  //     };

  //     _socket.async_receive_from(
  //         boost::asio::buffer(&session->_token, sizeof(session->_token)),
  //         session->_remote_endpoint,
  //         _strand.wrap(handle_receive));
  //   }

  //   /// See documentation at udp_session::enqueue_response.
  //   template <typename T>
  //   void enqueue_response(shared_session session, std::shared_ptr<T> data) {
  //     auto done = std::make_shared<std::atomic_bool>(false);

  //     post([session, data, done]() {
  //       while (!*done) {
  //         // here we split in packets and send.
  //         // _socket.send_to(packet.buffer(), session->_remote_endpoint);
  //       }
  //     });

  //     auto callback = [session, data, done]() {

  //     }

  //     _socket.async_send_to(
  //         boost::asio::buffer(data->buffer()),
  //         session->_remote_endpoint,
  //         _strand.wrap(handle_sent));
  //   }

  //   boost::asio::ip::udp::socket _socket;

  //   boost::asio::io_service::strand _strand;
  // };

////////////////////////////////////////////////////////////////////////////////

static void do_the_udp_thingy(size_t number_of_messages, size_t message_size) {
  auto out = util::sync_ostream(std::cout);

  constexpr auto payload = 65'000u;
  const auto packets_per_message = message_size / payload;
  const auto number_of_packets = number_of_messages * packets_per_message;

  out << "creating " << number_of_packets << " packets..." << std::endl;
  std::vector<const_shared_message> messages(number_of_packets);
  std::generate_n(messages.begin(), messages.size(), [=]() {
    return make_empty(payload);
  });

  using namespace boost::asio;
  using udp = boost::asio::ip::udp;

  constexpr auto port = 8080u;
  strm::detail::thread_group threads;

  // server.
  threads.create_thread([&](){
    out << "starting server..." << std::endl;

    io_service ios;
    udp::socket socket(ios, udp::endpoint(udp::v4(), port));


    auto i = 0u;
    for (auto x = 0u; x < number_of_messages; ++x) {
      STRM_PROFILE_FPS(server, send);

      uint32_t token;
      udp::endpoint remote_ep;
      socket.receive_from(buffer(&token, sizeof(token)), remote_ep);
      //out << "server: sending message " << token << std::endl;
      //std::this_thread::sleep_for(1000ns);
      for (auto y = 0u; y < packets_per_message; ++y) {
        socket.send_to(messages[i]->buffer(), remote_ep);
        socket.send_to(messages[i]->buffer(), remote_ep);
        ++i;
      }
    }

    out << "server is done!" << std::endl;
  });

  // client.
  io_service ios;
  udp::socket socket(ios);
  threads.create_thread([&](){
    out << "starting client..." << std::endl;

    udp::endpoint ep(udp::v4(), port);
    socket.open(ep.protocol());

    std::array<unsigned char, payload> buf;
    // auto i = 0u;
    for (auto x = 0u; x < number_of_messages; ++x) {
      STRM_PROFILE_FPS(client, receive);

      const uint32_t token = x;
      // out << "client: receiving message " << token << std::endl;
      socket.send_to(buffer(&token, sizeof(token)), ep);
      for (auto y = 0u; y < packets_per_message; ++y) {
        socket.receive_from(buffer(buf), ep);
      }
    }

    out << "client is done!" << std::endl;
  });

  // time-out.
  // std::this_thread::sleep_for(20s);
  // out << "closing down client." << std::endl;
  // socket.close();
  // ios.stop();
}

static void do_the_tcp_thingy(size_t number_of_messages, size_t message_size) {
  auto out = util::sync_ostream(std::cout);

  out << "creating " << number_of_messages << " messages..." << std::endl;
  std::vector<const_shared_message> messages(number_of_messages);
  std::generate_n(messages.begin(), messages.size(), [=]() {
    return make_empty(message_size);
  });

  using namespace boost::asio;
  using tcp = boost::asio::ip::tcp;

  strm::detail::thread_group threads;

  // server.
  threads.create_thread([&](){
    out << "starting tcp server..." << std::endl;

    io_service ios;
    tcp::acceptor acceptor(ios, tcp::endpoint(tcp::v4(), 8181u));
    auto socket = acceptor.accept();

    out << "tcp server connected" << std::endl;

    for (auto &&msg : messages) {
      STRM_PROFILE_FPS(server, send);
      socket.send(msg->buffer());
    }

    out << "tcp server done" << std::endl;
  });

  // client.
  threads.create_thread([&](){
    out << "starting tcp client..." << std::endl;

    io_service ios;
    tcp::socket s(ios);
    tcp::resolver resolver(ios);
    boost::asio::connect(s, resolver.resolve("127.0.0.1", "8181"));

    out << "tcp client connected" << std::endl;

    strm::message buf(message_size);
    auto count = 0u;
    while (true) {
      try {
        STRM_PROFILE_FPS(client, receive);
        __attribute__((unused)) size_t reply_length = boost::asio::read(s, buf.buffer());
        assert(reply_length == message_size);
        ++count;
      } catch (const std::exception &e) {
        out << "exception: " << e.what() << std::endl;
        break;
      }
    }

    out << "tcp client received " << count << " messages" << std::endl;
    out << "tcp client done" << std::endl;
  });
}

TEST(benchmark_, boost_udp_image_1920x1080) {
  do_the_udp_thingy(500u, 1920u * 1080u * 4u);
}

TEST(benchmark_, boost_tcp_image_1920x1080) {
  do_the_tcp_thingy(500u, 1920u * 1080u * 4u);
}

TEST(benchmark_, boost_udp_image_800x600) {
  do_the_udp_thingy(500u, 800u * 600u * 4u);
}

TEST(benchmark_, boost_tcp_image_800x600) {
  do_the_tcp_thingy(500u, 800u * 600u * 4u);
}

TEST(benchmark_, boost_udp_image_100x100) {
  do_the_udp_thingy(500u, 100u * 100u * 4u);
}

TEST(benchmark_, boost_tcp_image_100x100) {
  do_the_tcp_thingy(500u, 100u * 100u * 4u);
}
