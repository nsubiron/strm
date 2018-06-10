#include <strm/client.h>
#include <strm/server.h>

#include <strm/low_level/server_inspector.h>

#include "util/message.h"

#include <gtest/gtest.h>

#include <atomic>

using namespace std::chrono_literals;
using namespace util::message;
using inspector = strm::low_level::server_inspector;

TEST(integrity, inspector) {
  using datagram = strm::detail::udp_packet;
  constexpr auto payload = sizeof(datagram);

  strm::server srv(8080u);
  auto stream = srv.make_stream();
  srv.async_run(4u);

  ASSERT_EQ(inspector::payload(srv), payload);
  ASSERT_EQ(inspector::datagram_header_size(srv), 10u);
  ASSERT_EQ(inspector::datagram_capacity(srv), payload - inspector::datagram_header_size(srv));

  ASSERT_EQ(inspector::bytes_sent(srv), 0u);
  ASSERT_EQ(inspector::datagrams_sent(srv), 0u);

  std::atomic_size_t number_of_messages_received = 0u;
  strm::client client(stream.get_token());
  client.listen([&](shared_message){
    ++number_of_messages_received;
  });
  client.async_run(1u);

  auto msg = make_random(inspector::datagram_capacity(srv));
  stream << msg->buffer();

  std::this_thread::sleep_for(1s);

  srv.stop();
  client.stop();

  ASSERT_EQ(number_of_messages_received, 1u);
  ASSERT_EQ(inspector::bytes_sent(srv), payload);
  ASSERT_EQ(inspector::datagrams_sent(srv), 1u);
}

TEST(integrity, single_datagram) {
  constexpr auto number_of_messages_to_send = 10'000u;
  std::atomic_size_t number_of_messages_received = 0u;

  const std::string message = "Hello there!";

  strm::server server(8080u);
  strm::stream stream = server.make_stream();
  server.async_run(4u);

  strm::client client(stream.get_token());
  client.listen([&](shared_message m){
    ASSERT_EQ(as_string(*m), message);
    ++number_of_messages_received;
  });
  client.async_run(4u);

  std::this_thread::sleep_for(1s);

  for (auto i = 0u; i < number_of_messages_to_send; ++i) {
    stream << message;
  }

  std::this_thread::sleep_for(1s);

  server.stop();
  client.stop();

  ASSERT_EQ(inspector::datagrams_sent(server), number_of_messages_to_send);
  ASSERT_EQ(
      inspector::bytes_sent(server),
      number_of_messages_to_send * inspector::payload(server));
  ASSERT_EQ(number_of_messages_received, number_of_messages_to_send);
}

TEST(integrity, image_1920x1080) {
  constexpr auto message_size = 1920u * 1080u * 4u;
  constexpr auto number_of_messages_to_send = 100u;
  std::atomic_size_t number_of_messages_received = 0u;

  const auto message = make_random(message_size);

  strm::server server(8080u);
  strm::stream stream = server.make_stream();
  server.async_run(4u);

  strm::client client(stream.get_token());
  client.listen([&](shared_message result) {
    ASSERT_TRUE(*message == *result)
        << "\nmessage: " << *message
        << "\nresult:  " << *result;
    ++number_of_messages_received;
  });
  client.async_run(4u);

  std::this_thread::sleep_for(1s);

  for (auto i = 0u; i < number_of_messages_to_send; ++i) {
    stream << message->buffer();
  }

  std::this_thread::sleep_for(1s);

  server.stop();
  client.stop();

  auto expected_number_of_datagrams =
      1u + ((message_size - 1u) / inspector::datagram_capacity(server));
  expected_number_of_datagrams *= number_of_messages_to_send;
  ASSERT_EQ(inspector::datagrams_sent(server), expected_number_of_datagrams);
  ASSERT_EQ(
      inspector::bytes_sent(server),
      expected_number_of_datagrams * inspector::payload(server));
  ASSERT_EQ(number_of_messages_received, number_of_messages_to_send);
}

TEST(integrity, multiple_clients) {
  constexpr auto number_of_clients = 12u;
  constexpr auto number_of_messages_to_send = 100u;
  std::atomic_size_t number_of_messages_received = 0u;

  strm::server server(8080u);

  std::array<shared_message, number_of_clients> messages;
  std::vector<strm::stream> streams;
  std::vector<std::unique_ptr<strm::client>> clients;
  streams.reserve(number_of_clients);
  clients.reserve(number_of_clients);
  for (auto i = 0u; i < number_of_clients; ++i) {
    messages[i] = make_random(200u * 200u * 4u);
    streams.emplace_back(server.make_stream());
    clients.emplace_back(std::make_unique<strm::client>(streams.back().get_token()));
    clients.back()->listen([message=messages[i],&number_of_messages_received](shared_message result) {
      ASSERT_TRUE(*message == *result)
          << "\nmessage: " << *message
          << "\nresult:  " << *result;
      ++number_of_messages_received;
    });
    clients.back()->async_run(1u);
  }

  server.async_run(6u);

  // server.
  strm::detail::thread_group server_threads;
  for (auto i = 0u; i < number_of_clients; ++i) {
    auto stream = streams[i];
    auto message = messages[i];
    server_threads.create_thread([stream, message]() mutable {
      for (auto i = 0u; i < number_of_messages_to_send; ++i) {
        stream << message->buffer();
      }
    });
  }

  std::this_thread::sleep_for(5s);

  server.stop();
  for (auto &client : clients) {
    client->stop();
  }

  ASSERT_EQ(number_of_messages_received, number_of_messages_to_send * number_of_clients);
}
