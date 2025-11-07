#include <atomic>
#include <chrono>
#include <coroutine>
#include <iostream>
#include <source_location>  // ScopedLogger
#include <syncstream>
#include <thread>

// https://github.com/philsquared/cpp_coroutines_helpers/
// https://github.com/philsquared/understanding_cpp_coroutines

// Logs to cout, with current indent
template <typename... Args>
void log(Args&&... args);

struct ScopedLogger {
  inline static std::atomic<int> indent;

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
  std::osyncstream syncStream(std::cout);
  syncStream << std::string(ScopedLogger::indent, ' ');
  (syncStream << ... << std::forward<Args>(args));
}

// Creates a ScopedLogger for the current function
#define LOGF() ScopedLogger logger##__LINE__

using namespace std::chrono_literals;

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

    auto await_transform(std::chrono::milliseconds delay) {
      struct SleepFor {
        std::chrono::milliseconds MDelay;

        bool await_ready() const {
          LOGF();
          return MDelay == 0ms;
        }

        bool await_suspend(std::coroutine_handle<Task::promise_type> h) {
          LOGF();
          auto t = std::jthread([h, delay = MDelay]() {
            std::this_thread::sleep_for(delay);
            // Data race here
            h.resume();
          });
          t.detach();
          return true;
        }

        void await_resume() { LOGF(); }
      };
      return SleepFor{.MDelay = delay};
    }

    std::atomic<int> mReturnedValue{};
  };

  std::coroutine_handle<promise_type> handle;

  ~Task() {
    // Data race here
    // When commented - memory leak
    if (handle) {
      handle.destroy();
    }
  }

  auto operator=(Task&&) = delete;
  auto operator=(const Task&) = delete;

  int get_returned_value() const { return handle.promise().mReturnedValue; }
};

Task f() {
  LOGF();
  log(std::this_thread::get_id(), '\n');
  co_await 1000ms;
  log(std::this_thread::get_id(), '\n');
  co_return 42;
}

int main() {
  LOGF();
  auto task = f();
  log("Back in main(), sleeping ", std::this_thread::get_id(), '\n');
  std::this_thread::sleep_for(2000ms);
  log("ReturnedValue ", task.get_returned_value(), '\n');
  return 0;
}
