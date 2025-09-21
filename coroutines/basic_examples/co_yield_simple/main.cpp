#include <coroutine>
#include <iostream>
#include <source_location>  // ScopedLogger

// https://github.com/philsquared/cpp_coroutines_helpers/
// https://www.youtube.com/watch?v=b6pYieNd_OY
// C++ Coroutines - Gods from the Machine - Phil Nash - ACCU 2025

// Logs to cout, with current indent
template <typename... Args>
void log(Args&&... args);

struct ScopedLogger {
  inline static int indent;

  std::string m_name;

  ScopedLogger(
      std::string name = std::source_location::current().function_name())
      : m_name(std::move(name)) {
    log("\\ ", m_name, '\n');
    indent++;
  }
  ~ScopedLogger() {
    indent--;
    log("/ ", m_name, '\n');
  }
};

template <typename... Args>
void log(Args&&... args) {
  std::cout << std::string(ScopedLogger::indent, ' ');
  (std::cout << ... << std::forward<Args>(args));
}

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

    void return_value(int aValue) {
      LOGF();
      mReturnedValue = aValue;
    }

    auto yield_value(int aValue) {
      LOGF();
      mYieldedValue = aValue;
      // when suspend_never call to resume() is not needed
      return std::suspend_always{};
    }

    int mReturnedValue{};
    int mYieldedValue{};
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

  int get_returned_value() const { return handle.promise().mReturnedValue; }

  int get_yielded_value() const { return handle.promise().mYieldedValue; }

  void resume() { handle.resume(); }
};

Task f() {
  LOGF();
  co_yield 1;  // yield_value
  co_return 42;
}

int main() {
  LOGF();
  auto task = f();

  log("YieldedValue ", task.get_yielded_value(), '\n');

  // to get the co_return value when suspended
  task.resume();

  log("ReturnedValue ", task.get_returned_value(), '\n');
  return 0;
}
