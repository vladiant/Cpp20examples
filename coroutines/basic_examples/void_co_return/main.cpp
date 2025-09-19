// https://www.youtube.com/watch?v=b6pYieNd_OY
// C++ Coroutines - Gods from the Machine - Phil Nash - ACCU 2025

#include <coroutine>
#include <iostream>

struct Task {
  struct promise_type {
    Task get_return_object() { return {}; }
    auto initial_suspend() { return std::suspend_never{}; }
    // Almost always suspend from final suspend
    // Use suspend_never when no field in Task is accessed
    auto final_suspend() noexcept { return std::suspend_always{}; }
    void unhandled_exception() { std::abort(); }
    void return_void() {}
  };
};

Task f() { co_return; }

int main() {
  f();

  return 0;
}
