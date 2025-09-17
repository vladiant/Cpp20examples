#include <coroutine>
#include <iostream>
#include <source_location>  // ScopedLogger

// https://github.com/philsquared/cpp_coroutines_helpers/
// https://www.youtube.com/watch?v=b6pYieNd_OY
// C++ Coroutines - Gods from the Machine - Phil Nash - ACCU 2025

// Logs to cout, with current indent
#define LOG(...)                                                     \
  std::cout << std::string(ScopedLogger::indent, ' ') << __VA_ARGS__ \
            << std::endl

struct ScopedLogger {
  inline static int indent;

  std::string m_name;

  ScopedLogger(
      std::string name = std::source_location::current().function_name())
      : m_name(std::move(name)) {
    LOG("\\" << m_name);
    indent++;
  }
  ~ScopedLogger() {
    indent--;
    LOG("/" << m_name);
  }
};

// Creates a ScopedLogger for the current function
#define LOGF() ScopedLogger logger##__LINE__

struct Task {
  // lifetime tied to the coroutine
  struct promise_type {
    Task get_return_object() {
      LOGF();
      return {};
    }
    auto initial_suspend() {
      LOGF();
      // suspend_always will not trigger exception
      return std::suspend_never{};
    }
    auto final_suspend() noexcept {
      LOGF();
      return std::suspend_always{};
    }
    void unhandled_exception() {
      LOGF();
      std::abort();
    }
    void return_void() { LOGF(); }
  };
};

Task f() {
  LOGF();
  throw std::exception{};
  co_return;
}

int main() {
  LOGF();
  f();
  return 0;
}
