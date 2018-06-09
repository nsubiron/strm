#include <strm/client.h>
#include <strm/server.h>

#include <strm/detail/thread_group.h>

#include "util/message.h"
#include "util/stop_watch.h"

#include <gtest/gtest.h>

#include <iostream>

using namespace std::chrono_literals;

TEST(communication, single_datagram) {
  constexpr auto number_of_messages = 2000u;

  strm::server server(8080u);
  strm::stream stream = server.make_stream();
  server.async_run(2u);

  strm::client client(stream.get_token());
  client.listen([](std::shared_ptr<strm::message> m){
    std::string str(reinterpret_cast<const char*>(m->data()));
    ASSERT_EQ(str, "Hello!");
  });
  client.async_run(2u);

  for (auto i = 0u; i < 2u * number_of_messages; ++i) {
    stream << "Hello!";
  }

  std::this_thread::sleep_for(1s);

  server.stop();
  client.stop();
}

TEST(communication, big_message) {
  constexpr auto number_of_messages = 200u;
  const auto msg = util::message::make_random(1920u * 1080u * 4u);

  strm::server server(8080u);
  strm::stream stream = server.make_stream();
  server.async_run(2u);

  strm::client client(stream.get_token());
  client.listen([&](std::shared_ptr<strm::message>){
    // ASSERT_TRUE(std::equal(msg.begin(), msg.end(), m->data()));
  });
  client.async_run(2u);

  for (auto i = 0u; i < 2u * number_of_messages; ++i) {
    stream << msg.buffer();
  }

  std::this_thread::sleep_for(1s);

  server.stop();
  client.stop();
}

TEST(server, write_to_multiple_streams) {
  strm::detail::thread_group threads;

  constexpr auto number_of_clients = 12u;
  constexpr auto number_of_messages = 2000u;

  boost::asio::io_service io_service;
  strm::low_level::server server(io_service, 8080u);

  std::array<strm::token_type, number_of_clients> tokens;
  std::vector<strm::stream> streams;
  streams.reserve(number_of_clients);
  for (auto i = 0u; i < number_of_clients; ++i) {
    streams.emplace_back(server.make_stream());
    tokens[i] = streams[i].get_token();
  }

  auto make_message = [](size_t i){
    return std::string("Hello ") + std::to_string(i);
  };

  // server thread.
  threads.create_thread([=, &io_service]() mutable {
    std::this_thread::sleep_for(1s); // wait for clients.
    for (auto i = 0u; i < 2u * number_of_messages; ++i) {
      for (auto j = 0u; j < number_of_clients; ++j) {
        std::string message = make_message(j);
        streams[j] << message;
      }
    }
    std::this_thread::sleep_for(1s);
    io_service.stop();
  });

  // for (auto j = 0u; j < number_of_clients; ++j) {
  //   // client thread.
  //   auto token = tokens[j];
  //   threads.create_thread([=, &io_service](){
  //     strm::low_level::client client(io_service, token);
  //     // for (auto i = 0u; i < number_of_messages; ++i) {
  //     //   strm::detail::udp_packet packet;
  //     //   ASSERT_TRUE(client.read_packet(packet));
  //     //   std::string message(
  //     //       static_cast<const char *>(packet.data()),
  //     //       packet.size());
  //     //   ASSERT_EQ(message, make_message(j));
  //     // }
  //   });
  // }

  threads.create_threads(4u, [&](){ io_service.run(); });

  io_service.run();
}
