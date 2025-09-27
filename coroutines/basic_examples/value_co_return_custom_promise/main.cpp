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

class MyPromiseType;

struct Task {
  friend class MyPromiseType;
  std::coroutine_handle<MyPromiseType> handle;

  ~Task() {
    if (handle) {
      handle.destroy();
    }
  }

  auto operator=(Task&&) = delete;
  auto operator=(const Task&) = delete;

  void resume() {
    LOGF();
    handle.resume();
  }

  int get_returned_value() const;
};

struct MyPromiseType {
  MyPromiseType(Task& task, int) {
    task.handle = std::coroutine_handle<MyPromiseType>::from_promise(*this);
  }

  void get_return_object() { LOGF(); }

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

  void return_value(int aValue) {
    LOGF();
    mValue = aValue;
  }

  int mValue{};
};

// MyPromiseType definition needed here
int Task::get_returned_value() const { return handle.promise().mValue; }

template <>
struct std::coroutine_traits<void, Task&, int> {
  using promise_type = MyPromiseType;
};

void f(Task& task, int n) {
  LOGF();
  co_return n;
}

int main() {
  LOGF();
  Task task;
  f(task, 72);
  task.resume();
  log("ReturnedValue ", task.get_returned_value(), '\n');
  return 0;
}
