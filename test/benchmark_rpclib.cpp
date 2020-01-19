// These two need to be the first includes.
#include "util/profiler.h"
#include <gtest/gtest.h>

#include <strm/detail/thread_group.h>

#include <rpc/client.h>
#include <rpc/server.h>

#include "util/message.h"
#include "util/sync_ostream.h"

using namespace std::chrono_literals;
using namespace util::message;

static void do_the_thing(size_t number_of_messages, size_t message_size) {
  auto out = util::sync_ostream(std::cout);

  out << "creating " << number_of_messages << " packets..." << std::endl;
  std::vector<std::vector<unsigned char>> messages(number_of_messages);
  std::generate_n(messages.begin(), messages.size(), [=]() {
    return std::vector<unsigned char>(message_size);
  });

  for (auto i = 0u; i < number_of_messages; ++i) {
    assert(messages.at(i).size() == message_size);
  }

  constexpr auto port = 8080u;
  rpc::server srv(port);
  srv.bind("get_image", [&](size_t i) {
    STRM_PROFILE_FPS(server, send);
    return messages[i];
  });
  srv.async_run(1u);

  strm::detail::thread_group threads;
  threads.create_thread([&]() {
    rpc::client c("localhost", 8080);
    for (auto i = 0u; i < number_of_messages; ++i) {
      STRM_PROFILE_FPS(client, receive);
      auto msg = c.call("get_image", i).as<std::vector<unsigned char>>();
      // ASSERT_EQ(msg, messages[i]);
    }
  });
}


TEST(benchmark_, rpc_image_1920x1080) {
  do_the_thing(500u, 1920u * 1080u * 4u);
}

TEST(benchmark_, rpc_image_800x600) {
  do_the_thing(500u, 800u * 600u * 4u);
}

TEST(benchmark_, rpc_image_100x100) {
  do_the_thing(500u, 100u * 100u * 4u);
}
