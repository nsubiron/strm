#include <strm/server.h>
#include <strm/client.h>

#include "stop_watch.h"
#include "thread_group.h"

#include <gtest/gtest.h>

#include <iostream>

using namespace std::chrono_literals;

TEST(server, write_to_stream) {
  thread_group threads;

  constexpr auto number_of_messages = 2000u;

  boost::asio::io_service io_service;
  strm::server server(io_service, 8080u);
  strm::stream stream = server.make_stream();

  const auto token = stream.get_token();

  // server thread.
  threads.create_thread([stream, &io_service]() mutable {
    std::this_thread::sleep_for(1s); // wait for client.
    for (auto i = 0u; i < 2u * number_of_messages; ++i) {
      stream << "Hello!";
    }
    std::this_thread::sleep_for(1s);
    io_service.stop();
  });

  // client thread.
  threads.create_thread([token, &io_service](){
    strm::client client(io_service, 8080u);
    client.subscribe_to_stream(token);
    for (auto i = 0u; i < number_of_messages; ++i) {
      strm::udp_packet packet;
      ASSERT_TRUE(client.read_packet(packet));
      std::string message(static_cast<const char *>(packet.data()));
      ASSERT_EQ(message, "Hello!");
    }
  });

  threads.create_threads(4u, [&](){ io_service.run(); });

  io_service.run();
}

TEST(server, write_to_multiple_streams) {
  thread_group threads;

  constexpr auto number_of_clients = 12u;
  constexpr auto number_of_messages = 2000u;

  boost::asio::io_service io_service;
  strm::server server(io_service, 8080u);

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

  for (auto j = 0u; j < number_of_clients; ++j) {
    // client thread.
    auto token = tokens[j];
    threads.create_thread([=, &io_service](){
      strm::client client(io_service, 8080u);
      client.subscribe_to_stream(token);
      for (auto i = 0u; i < number_of_messages; ++i) {
        strm::udp_packet packet;
        ASSERT_TRUE(client.read_packet(packet));
        std::string message(
            static_cast<const char *>(packet.data()),
            packet.size());
        ASSERT_EQ(message, make_message(j));
      }
    });
  }

  threads.create_threads(4u, [&](){ io_service.run(); });

  io_service.run();
}
