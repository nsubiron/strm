#include "profiler.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>

namespace util {

  static std::string PROFILER_LOG_FILENAME;

  // https://stackoverflow.com/a/27375675
  template <typename Arg, typename ... Args>
  static void write_to_stream(std::ostream &out, Arg &&arg, Args &&... args) {
    out << std::boolalpha
        << std::left << std::setw(44)
        << std::forward<Arg>(arg)
        << std::right
        << std::fixed << std::setprecision(2);
    using expander = int[];
    (void)expander{0, (void(out << ", " << std::setw(10) << std::forward<Args>(args)),0)...};
  }

  template <typename ... Args>
  static void write_to_file(std::ios_base::openmode mode, Args &&... args) {
    if (!PROFILER_LOG_FILENAME.empty()) {
      static std::mutex MUTEX;
      std::lock_guard<std::mutex> guard(MUTEX);
      std::ofstream file(PROFILER_LOG_FILENAME, mode);
      write_to_stream(file, std::forward<Args>(args)...);
      file << std::endl;
    }
  }

  template <typename ... Args>
  static void write_line(Args &&... args) {
    write_to_file(std::ios_base::app|std::ios_base::out, std::forward<Args>(args)...);
  }

  static std::string get_date() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d%H%M%S");
    return oss.str();
  }

  void profiler::initialize(int argc, char **argv) {
    PROFILER_LOG_FILENAME = argc > 1 ? std::string(argv[1u]) : get_date();
    std::cout << "Writing profiling data to " << PROFILER_LOG_FILENAME << '\n';
    write_to_file(std::ios_base::out,
        "# context",
        "average",
        "maximum",
        "minimum",
        "units",
        "times");
  }

namespace detail {

  profiler_data::~profiler_data() {
    if (_count > 0u) {
      if (_print_fps) {
        write_line(_name, fps(average()), fps(minimum()), fps(maximum()), "FPS", _count);
      } else {
        write_line(_name, average(), maximum(), minimum(), "ms", _count);
      }
    } else {
      std::cerr << "profiler " << _name << " was never run!\n";
    }
  }

} // namespace detail
} // namespace util
