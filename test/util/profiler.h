#pragma once

#include "stop_watch.h"

#include <algorithm>
#include <string>

namespace util {

  class profiler {
  public:

    static void initialize(int argc, char **argv);
  };

namespace detail {

  class profiler_data {
  public:

    explicit profiler_data(std::string name, bool print_fps = false)
      : _name(std::move(name)),
        _print_fps(print_fps) {}

    ~profiler_data();

    void annotate(const stop_watch &stop_watch) {
      size_t elapsed_microseconds = stop_watch.GetElapsedTime<std::chrono::microseconds>();
      ++_count;
      _total_microseconds += elapsed_microseconds;
      _max_elapsed = std::max(elapsed_microseconds, _max_elapsed);
      _min_elapsed = std::min(elapsed_microseconds, _min_elapsed);
    }

    float average() const {
      return ms(_total_microseconds) / static_cast<float>(_count);
    }

    float maximum() const {
      return ms(_max_elapsed);
    }

    float minimum() const {
      return ms(_min_elapsed);
    }

  private:

    static inline float ms(size_t microseconds) {
      return 1e-3f * static_cast<float>(microseconds);
    }

    static inline float fps(float milliseconds) {
      return 1e3f / milliseconds;
    }

    const std::string _name;

    const bool _print_fps;

    size_t _count = 0u;

    size_t _total_microseconds = 0u;

    size_t _max_elapsed = 0u;

    size_t _min_elapsed = -1;
  };

  class scoped_profiler {
  public:

    explicit scoped_profiler(profiler_data &parent) : _profiler(parent) {}

    ~scoped_profiler() {
      _stop_watch.Stop();
      _profiler.annotate(_stop_watch);
    }

  private:

    profiler_data &_profiler;

    stop_watch _stop_watch;
  };

} // namespace detail
} // namespace util

#define STRM_GTEST_GET_TEST_NAME() std::string(::testing::UnitTest::GetInstance()->current_test_info()->name())

#define STRM_PROFILE_SCOPE(context, profiler_name) \
    static thread_local ::util::detail::profiler_data strm_profiler_ ## context ## _ ## profiler_name ## _data( \
        STRM_GTEST_GET_TEST_NAME() + "." #context "." #profiler_name); \
    ::util::detail::scoped_profiler strm_profiler_ ## context ## _ ## profiler_name ## _scoped_profiler( \
        strm_profiler_ ## context ## _ ## profiler_name ## _data);

#define STRM_PROFILE_FPS(context, profiler_name) \
    { \
      static thread_local ::util::stop_watch stop_watch; \
      stop_watch.Stop(); \
      static thread_local bool first_time = true; \
      if (!first_time) { \
        static thread_local ::util::detail::profiler_data profiler_data( \
            STRM_GTEST_GET_TEST_NAME() + "." #context "." #profiler_name, true); \
        profiler_data.annotate(stop_watch); \
      } else { \
        first_time = false; \
      } \
      stop_watch.Restart(); \
    }
