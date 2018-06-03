#include <strm/server.h>

#include "stop_watch.h"

#include <gtest/gtest.h>

#include <iostream>

TEST(server, write_to_stream) {
  boost::asio::io_service io_service;

  strm::server srv(io_service, 8080u);

  auto stream = srv.make_stream();

  stream << "Hello world!";

  // io_service.run();
}
