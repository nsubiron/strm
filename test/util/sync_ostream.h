#pragma once

#include <mutex>
#include <ostream>
#include <sstream>

namespace util {

  class sync_ostream {
  public:

    explicit sync_ostream(std::ostream &out) : _out(out) {}

    ~sync_ostream() {
      flush();
    }

    template <typename T>
    sync_ostream &operator<<(const T &obj) {
      _buffer << obj;
      return *this;
    }

    sync_ostream &operator<<( std::ostream&(*f)(std::ostream&) ) {
      _buffer << f;
      if(f == (std::ostream& (*)(std::ostream&)) &std::endl) {
        flush();
      }
      return *this;
    }

    void flush();

  private:

    std::mutex _mutex;

    std::ostream &_out;

    static thread_local std::ostringstream _buffer;
  };

} // namespace util
