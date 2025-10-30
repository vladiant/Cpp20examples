#include <cassert>
#include <coroutine>
#include <iostream>
#include <memory_resource>
#include <source_location>  // ScopedLogger
#include <utility>

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

thread_local std::pmr::memory_resource* current_resource = nullptr;

struct Task {
  struct promise_type {
    Task get_return_object() {
      LOGF();
      return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    auto initial_suspend() {
      LOGF();
      return std::suspend_always{};
    }

    auto final_suspend() noexcept {
      LOGF();
      return std::suspend_always{};
    }

    void unhandled_exception() {
      LOGF();
      std::abort();
    }

    auto yield_value(int aValue) {
      LOGF();
      mYieldedValue = aValue;
      return std::suspend_always{};
    }

    void return_void() { LOGF(); }

    int mYieldedValue{};

    void* operator new(std::size_t sz) {
      LOGF();
      log("size: ", sz, '\n');
      assert(current_resource && "PMR resource not set!");
      return current_resource->allocate(sz);
    }

    void operator delete(void* ptr, std::size_t sz) {
      LOGF();
      current_resource->deallocate(ptr, sz);
    }
  };

  // Cotrols and destroys coroutines
  // Obtained by a promise
  std::coroutine_handle<promise_type> handle;

  ~Task() {
    LOGF();
    if (handle) {
      handle.destroy();
    }
  }

  auto operator=(Task&&) = delete;
  auto operator=(const Task&) = delete;

  int get_yielded_value() const { return handle.promise().mYieldedValue; }

  // Updated to verify there are values to yield
  bool resume() {
    handle.resume();
    return handle.done();
  }
};

Task f(int repetitions, int extra) {
  LOGF();
  int a = 0;
  int b = 1;
  co_yield a;
  while (repetitions-- > 0) {
    co_yield (a = std::exchange(b, a + b)) + extra;
  }
}

int main() {
  LOGF();
  char buffer[1];
  std::pmr::monotonic_buffer_resource mem_res(buffer, sizeof(buffer));
  current_resource = &mem_res;

  auto task = f(5, 1);

  while (!task.resume()) {
    log("YieldedValue ", task.get_yielded_value(), '\n');
  };

  return 0;
}
