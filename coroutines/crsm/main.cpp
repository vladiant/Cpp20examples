#include <coroutine>
#include <iostream>

// Coroutine State Machine 

// Compiles with: GCC 11.1 (GCC 10.2 -fcoroutines), Clang 15.0.0, msvc 19.30

class [[nodiscard]] CoState {
 public:
  struct promise_type;
  using CoroHandle = std::coroutine_handle<promise_type>;

  // We use this class to deal on pass execution to the
  // latest function called on the end of the coroutine
  struct FinalAwaiter {
    constexpr bool await_ready() const noexcept { return false; }

    std::coroutine_handle<> await_suspend(CoState::CoroHandle hnd) noexcept {
      // the coroutine is now suspended at the final suspend point
      // - resume its continuation if there is one
      if (hnd.promise().nextHandle) {
        return hnd.promise().nextHandle;  // return the next to resume
      } else {
        return std::noop_coroutine();  // no next => return to caller with a
                                       // special handle
      }
    }

    void await_resume() const noexcept {}
  };

  struct promise_type {
    CoState get_return_object() { return {CoroHandle::from_promise(*this)}; }

    std::suspend_always initial_suspend() const noexcept { return {}; }

    FinalAwaiter final_suspend() const noexcept { return {}; }

    void unhandled_exception() { std::terminate(); }

    void return_void() const noexcept {}

    CoState::CoroHandle nextHandle = nullptr;
  };

 public:
  CoState(CoroHandle h) : hnd{h} {}

  ~CoState() {
    if (hnd) {
      hnd.destroy();
    }
  }

  CoState(const CoState&) = delete;
  CoState& operator=(const CoState& state) = delete;

  bool start() {
    if (!hnd || hnd.done()) {
      return false;
    }

    hnd.resume();
    return !hnd.done();
  }

 public:
  // Awaiter
  constexpr bool await_ready() const noexcept { return false; }

  std::coroutine_handle<> await_suspend(CoroHandle caller) noexcept {
    hnd.promise().nextHandle = caller;
    return hnd;
  }

  constexpr void await_resume() const noexcept {}

 private:
  CoroHandle hnd;
};

CoState state1(int num);
CoState state2(int num);
CoState state3(int num);

CoState state1(int num) {
  std::cout << "State 1 " << num << "\n";
  if (num > 3) {
    co_await state2(num - 3);
  } else {
    co_await state3(7);
  }
}

CoState state2(int num) {
  std::cout << "State 2 " << num << "\n";
  co_await state1(num - 2);
}

CoState state3(int num) {
  std::cout << "State 3 " << num << "\n";
  if (num > 0) {
    co_await state3(num - 2);
  }
}

int main() {
  CoState machine = state1(10);

  machine.start();  // Start and wait for the end of execution

  return 0;
}
