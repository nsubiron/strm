// These two need to be the first includes.
#include "util/profiler.h"
#include <gtest/gtest.h>

#include <strm/client.h>
#include <strm/server.h>

#include <strm/detail/thread_group.h>

#include "util/message.h"

using namespace std::chrono_literals;

TEST(benchmark, single_datagram) {
  constexpr auto number_of_messages = 10'000u;

  strm::server server(8080u);
  strm::stream stream = server.make_stream();
  server.async_run(1u);

  strm::client client(stream.get_token());
  client.listen([](std::shared_ptr<strm::message>){
    STRM_PROFILE_FPS(client, listen_callback);
  });
  client.async_run(1u);

  {
    for (auto i = 0u; i < number_of_messages; ++i) {
      STRM_PROFILE_SCOPE(game, write_to_stream);
      stream << "Hello!";
    }
  }

  std::this_thread::sleep_for(1s);

  server.stop();
  client.stop();
}

static void benchmark_image(size_t dimensions) {
  constexpr auto number_of_messages = 20u;

  strm::server server(8080u);
  strm::stream stream = server.make_stream();
  server.async_run(1u);

  strm::client client(stream.get_token());
  client.listen([](std::shared_ptr<strm::message>){
    STRM_PROFILE_FPS(client, listen_callback);
  });
  client.async_run(1u);
  std::this_thread::sleep_for(2s);

  strm::detail::thread_group threads;
  threads.create_thread([=]() mutable {
    const auto msg = util::message::make_random(dimensions * 4u);
    for (auto i = 0u; i < number_of_messages; ++i) {
      STRM_PROFILE_SCOPE(game, write_to_stream);
      stream << msg->buffer();
    }
  });

  std::this_thread::sleep_for(5s);

  threads.join_all();
  server.stop();
  client.stop();
}

TEST(benchmark, image_1920x1080) {
  benchmark_image(1920u * 1080u);
}

TEST(benchmark, image_800x600) {
  benchmark_image(800u * 600u);
}
