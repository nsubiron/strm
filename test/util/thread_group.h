#pragma once

#include <thread>
#include <vector>

namespace util {

  class thread_group {
  public:

    thread_group() {}

    thread_group(const thread_group &) = delete;
    thread_group &operator=(const thread_group &) = delete;

    ~thread_group() { join_all(); }

    template <typename F>
    void create_thread(F &&functor) {
      _threads.emplace_back(std::forward<F>(functor));
    }

    template <typename F>
    void create_threads(size_t count, F functor) {
      _threads.reserve(_threads.size() + count);
      for (size_t i = 0u; i < count; ++i) {
        create_thread(functor);
      }
    }

    void join_all() {
      for (auto &thread : _threads) {
        if (thread.joinable()) {
          thread.join();
        }
      }
    }

  private:

    std::vector<std::thread> _threads;
  };

} // namespace util
