#include "sync_ostream.h"

namespace util {

  static const char *make_color() {
    static constexpr char palette[][5u] = {
      "0;34", "0;35", "0;36", "1;34", "1;35", "1;36", "1;37", "1;30"
    };
    static auto index = 0u;
    return palette[index++ % std::size(palette)];
  }

  void sync_ostream::flush() {
    static thread_local auto color = make_color();
    {
      std::lock_guard<std::mutex> guard(_mutex);
      _out << "\x1B[" << color << "m" << _buffer.str() << "\x1B[0m";
    }
    _buffer.clear();
    _buffer.str("");
  }

  thread_local std::ostringstream sync_ostream::_buffer;

} // namespace util
