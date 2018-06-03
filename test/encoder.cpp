#include <strm/encoder.h>

#include "message.h"
#include "stop_watch.h"

#include <gtest/gtest.h>

#include <iostream>
#include <memory>
#include <random>

TEST(encoder, big_message) {
  using udp_packet = strm::udp_packet;

  auto message = message::make_random(1920u * 1080u * 4u);

  const auto expected_number_of_packets =
      1u + ((message.size() - 1u) / udp_packet::numeric_limits::packet_capacity());

  std::vector<udp_packet> packets;
  packets.reserve(expected_number_of_packets);

  // encode.
  {
    strm::encoder enc;
    StopWatch sw;

    auto splitter = enc.split_message(message.buffer());
    ASSERT_GT(splitter.number_of_packets(), 0u);
    ASSERT_EQ(splitter.number_of_packets(), expected_number_of_packets);
    for (auto &packet : splitter) {
      packets.emplace_back(packet);
    }

    sw.Stop();
    ASSERT_EQ(packets.size(), splitter.number_of_packets());
    std::cout << "encoding " << splitter << " took " << sw.GetElapsedTime() << "ms\n";
  }

  // shuffle data.
  {
    std::random_device rd;
    std::minstd_rand g(rd());
    std::shuffle(packets.begin(), packets.end(), g);
  }

  // decode.
  message::Message result;
  {
    const auto message_size = packets[0u].message_size();
    ASSERT_EQ(packets[0u].total_number_of_packets(), packets.size());

    result = message::make_empty(message_size);
    unsigned char * const begin = result.data();

    for (const auto &pack : packets) {
      const auto i = pack.packet_number();
      constexpr size_t packet_capacity = udp_packet::numeric_limits::packet_capacity();
      ASSERT_LT((i * packet_capacity), message_size);
      void *dest = begin + (i * packet_capacity);
      std::memcpy(dest, pack.data(), pack.size());
    }
  }

  ASSERT_TRUE(result == message)
      << "\nmessage: " << message
      << "\nresult:  " << result;
}
