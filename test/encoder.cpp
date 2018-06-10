#include <strm/detail/decoder.h>
#include <strm/detail/encoder.h>

#include "util/message.h"
#include "util/stop_watch.h"

#include <gtest/gtest.h>

#include <iostream>
#include <memory>
#include <random>

using udp_datagram = strm::detail::udp_packet;
using namespace util::message;

static std::vector<udp_datagram> encode(boost::asio::const_buffer buffer) {
  strm::detail::encoder enc;
  std::vector<udp_datagram> packets;
  auto splitter = enc.split_message(buffer);
  packets.reserve(splitter.number_of_packets());
  for (auto &packet : splitter) {
    packets.emplace_back(packet);
  }
  EXPECT_EQ(packets.size(), splitter.number_of_packets());
  return packets;
}

static shared_message decode(const std::vector<udp_datagram> &packets) {
  constexpr auto PAYLOAD = 512u;
  static_assert(PAYLOAD == sizeof(udp_datagram), "Invalid datagram size!");
  shared_message result;
  strm::detail::decoder dec(PAYLOAD, [&result](shared_message msg){
    result = std::move(msg);
  });
  for (auto &datagram : packets) {
    dec.parse(reinterpret_cast<const unsigned char *>(&datagram), sizeof(datagram));
  }
  EXPECT_NE(result, nullptr);
  return result;
}

static void shuffle(std::vector<udp_datagram> &packets) {
  std::random_device rd;
  std::minstd_rand g(rd());
  std::shuffle(packets.begin(), packets.end(), g);
}

TEST(encoder, single_datagram) {
  for (auto i = 0u; i < 10'000u; ++i) {
    std::string message = "Hello!";
    auto packets = encode(boost::asio::buffer(message));
    ASSERT_EQ(packets.size(), 1u);
    auto result = decode(packets);
    ASSERT_EQ(message, as_string(*result));
  }
}

TEST(encoder, image_1920x1080) {
  constexpr auto message_size = 1920u * 1080u * 4u;
  constexpr auto expected_number_of_packets =
      1u + ((message_size - 1u) / udp_datagram::numeric_limits::packet_capacity());

  for (auto i = 0u; i < 10u; ++i) {
    auto message = make_random(message_size);
    auto packets = encode(message->buffer());
    ASSERT_EQ(packets.size(), expected_number_of_packets);
    shuffle(packets);
    auto result = decode(packets);
    ASSERT_TRUE(*message == *result)
        << "\nmessage: " << *message
        << "\nresult:  " << *result;
  }
}
