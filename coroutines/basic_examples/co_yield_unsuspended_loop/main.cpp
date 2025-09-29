#include <coroutine>
#include <iostream>
#include <queue>
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
      mYieldedValues.push(aValue);
      return std::suspend_never{};
    }

    int mReturnedValue{};
    std::queue<int> mYieldedValues;
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

  auto get_yielded_value() const { return handle.promise().mYieldedValues; }

  // Updated to verify there are values to yield
  bool done() { return handle.done(); }
};

Task f() {
  LOGF();
  co_yield 1;
  co_yield 2;
  co_yield 3;
  co_return 42;
}

int main() {
  LOGF();
  auto task = f();

  std::queue<int> yieldedValues;
  do {
    yieldedValues = task.get_yielded_value();
    log("Yielded values count: ", yieldedValues.size(), '\n');
  } while (!task.done());

  while (!yieldedValues.empty()) {
    log("Yielded value: ", yieldedValues.front(), '\n');
    yieldedValues.pop();
  }

  log("ReturnedValue ", task.get_returned_value(), '\n');
  return 0;
}
