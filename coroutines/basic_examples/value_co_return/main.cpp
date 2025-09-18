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
  struct promise_type {
    Task get_return_object() {
      LOGF();
      return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    auto initial_suspend() {
      LOGF();
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

    // void return_void() should not be used alongside return_value
    void return_value(int aValue) {
      LOGF();
      mValue = aValue;
    }

    int mValue{};
  };

  // Cotrols and destroys coroutines
  // Obtained by a promise
  std::coroutine_handle<promise_type> handle;

  ~Task() {
    if (handle) {
      handle.destroy();
    }
  }

  auto operator=(Task&&) = delete;
  auto operator=(const Task&) = delete;

  int get_returned_value() const { return handle.promise().mValue; }
};

Task f() {
  LOGF();
  co_return 42;  // return_value
}

int main() {
  LOGF();
  auto task = f();
  LOG(task.get_returned_value());
  return 0;
}
