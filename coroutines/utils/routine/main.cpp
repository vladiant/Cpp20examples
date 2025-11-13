#include <coroutine>
#include <cstdio>

// No need to modify
struct Routine {};

// struct Routine {
//    // The return type has to contain a promise_type
//    using promise_type = RoutinePromise;
//};

struct RoutinePromise {
  // This function is used to create the instance
  // of the return type for the caller
  Routine get_return_object() { return {}; }

  // What should happen before the coroutine body starts
  std::suspend_never initial_suspend() noexcept { return {}; }
  // What should happen after the coroutine body has finished
  std::suspend_never final_suspend() noexcept { return {}; }
  // What should happen when the coroutine executes co_return;
  void return_void() {}
  // What should happen when there is an unhandled exception
  void unhandled_exception() {}
};

// Any coroutine returning Routine
template <typename... Arg>
struct std::coroutine_traits<Routine, Arg...> {
  using promise_type = RoutinePromise;
};

int main() {
  auto coro = [] -> Routine {
    std::puts("coro running...");
    co_return;
  };

  auto x = coro();  // coroutine starts and runs to completion
  // decltype(x) == Routine
  static_assert(std::is_same_v<decltype(x), Routine>);

  coro();  // Because the return type is empty, this is the same as above
}
