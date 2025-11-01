#include <coroutine>
#include <iostream>
#include <source_location>  // ScopedLogger

// https://github.com/philsquared/cpp_coroutines_helpers/
// https://github.com/philsquared/understanding_cpp_coroutines

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

    int mReturnedValue{};
    int mAwaitedValue{};
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

  void set_awaited_value(int aValue) const {
    handle.promise().mAwaitedValue = aValue;
  }
};

struct MyAwiatable {
  bool await_ready() const {
    LOGF();
    // instantly ready
    return true;
  }

  void await_suspend(std::coroutine_handle<Task::promise_type> h) {
    LOGF();
    handle = h;
  }

  int await_resume() {
    LOGF();
    if (handle) {
      return handle.promise().mAwaitedValue;
    }
    return -1;
  }

 private:
  std::coroutine_handle<Task::promise_type> handle;
};

Task f() {
  LOGF();
  int value = co_await MyAwiatable{};
  log("AwaitedValue ", value, '\n');
  co_return 42;
}

int main() {
  LOGF();
  auto task = f();
  // not waited
  task.set_awaited_value(7);
  log("ReturnedValue ", task.get_returned_value(), '\n');
  return 0;
}
