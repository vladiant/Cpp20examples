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
    promise_type() { LOGF(); }

    ~promise_type() { LOGF(); }

    Task get_return_object() {
      LOGF();
      return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    auto initial_suspend() {
      LOGF();
      // Should be suspend_always to prevent access to destroyed object
      return std::suspend_always{};
    }

    auto final_suspend() noexcept {
      LOGF();
      // No problem with void co_return
      return std::suspend_never{};
    }

    void unhandled_exception() {
      LOGF();
      std::abort();
    }

    void return_value(int aValue) {
      LOGF();
      mValue = aValue;
    }

    int mValue{};
  };

  // Cotrols and destroys coroutines
  // Obtained by a promise
  std::coroutine_handle<promise_type> handle;

  Task(std::coroutine_handle<promise_type> aHandle) : handle{aHandle} {
    LOGF();
  }

  ~Task() {
    LOGF();
    if (handle) {
      handle.destroy();
    }
  }

  auto operator=(Task&&) = delete;
  auto operator=(const Task&) = delete;

  int get_returned_value() const {
    if (handle) {
      return handle.promise().mValue;
    }
    return -1;
  }
};

Task f() {
  LOGF();
  // final_suspend is suspend_never
  // return value never set!!!
  co_return 42;  // return_value
}

int main() {
  LOGF();
  auto task = f();

  log(task.get_returned_value(), '\n');
  return 0;
}
