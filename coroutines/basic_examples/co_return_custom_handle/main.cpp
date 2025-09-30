#include <coroutine>
#include <iostream>
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

template <typename PromiseType>
struct ScopedCoroHandle : std::coroutine_handle<PromiseType> {
  std::coroutine_handle<PromiseType>& asHandle() {
    return static_cast<std::coroutine_handle<PromiseType>&>(*this);
  }

  ScopedCoroHandle() = default;
  ScopedCoroHandle(std::coroutine_handle<PromiseType> aHandle)
      : std::coroutine_handle<PromiseType>{aHandle} {}

  ScopedCoroHandle& operator=(ScopedCoroHandle&& other) = delete;

  ~ScopedCoroHandle() {
    if (*this) {
      this->destroy();
    }
  }
};

template <typename PromiseType>
struct UniqueCoroHandle : ScopedCoroHandle<PromiseType> {
  using ScopedCoroHandle<PromiseType>::ScopedCoroHandle;

  UniqueCoroHandle(UniqueCoroHandle&& other) noexcept
      : ScopedCoroHandle<PromiseType>{std::exchange(
            other.asHandle(), std::coroutine_handle<PromiseType>())} {}

  UniqueCoroHandle& operator=(UniqueCoroHandle&& other) noexcept {
    if (*this != other) {
      if (*this) {
        this->destroy();
      }
      this->asHandle() =
          std::exchange(other.asHandle(), std::coroutine_handle<PromiseType>());
    }
    return *this;
  }
};

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

    void unhandled_exception() { LOGF(); }

    void return_void() { LOGF(); }
  };

  UniqueCoroHandle<promise_type> handle;

  void resume() { handle.resume(); }
};

Task f() {
  LOGF();
  co_return;
}

int main() {
  LOGF();
  {
    auto task = f();
    auto&& task2 = std::move(task);

    Task task3{{}};
    task3 = std::move(task2);
    task3 = std::move(task3);

    Task task4 = f();
    task3 = std::move(task4);
  }

  return 0;
}
