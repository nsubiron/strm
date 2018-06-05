#include <strm/stream.h>
#include <strm/server.h>

#include <gtest/gtest.h>

#include <array>
#include <iostream>
#include <list>
#include <set>
#include <string>
#include <vector>

TEST(stream, compile) {
  strm::server srv(8080u);
  auto stream = srv.make_stream();

  { std::array<boost::asio::const_buffer, 3u> s; stream.write(s); }
  { std::vector<boost::asio::const_buffer> s; stream.write(s); }
  { std::list<boost::asio::const_buffer> s; stream.write(s); }
  { std::set<boost::asio::const_buffer> s; stream.write(s); }

  { boost::asio::const_buffer v; stream << v; }
  { boost::asio::mutable_buffer v; stream << v; }
  { int v[42u]; stream << v; }
  { std::vector<int> v; stream << v; }
  { std::string v; stream << v; }
  { std::wstring v; stream << v; }
  { struct C { int x = 0; } v; stream << boost::asio::buffer(&v, sizeof(v)); }
  { struct C { int x = 0; } v[42u]; stream << v; }
  { struct C { int x = 0; }; std::array<C, 42u> v; stream << v; }
  { struct C { int x = 0; }; std::vector<C> v; stream << v; }
}
