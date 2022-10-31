#include <coroutine>
#include <iostream>

// Compiles with: GCC 11.1 (GCC 10.2 -fcoroutines), Clang 15.0.0, msvc 19.30

class [[nodiscard]] CoroSimple {
 public:
  // Having such an internal struct is required
  struct promise_type;
  using CoroHandle = std::coroutine_handle<promise_type>;

 public:
  // This constructor is used from
  //  CoroSimple::promise_type::get_return_object.
  CoroSimple(CoroHandle h) : hnd{h} { std::cout << "CoroSimple\tctor\n"; }

  // We have to deal with resources
  ~CoroSimple() {
    std::cout << "CoroSimple\tdtor\n";
    if (hnd) {
      hnd.destroy();
    }
  }

  // copying and moving is non-trivial,
  //  so we prevent it here for simplicity
  CoroSimple(const CoroSimple&) = delete;
  CoroSimple& operator=(const CoroSimple&) = delete;

 public:
  // A public method to interact with this handle
  bool resume() const {
    std::cout << "CoroSimple\tresume\n";
    if (!hnd || hnd.done()) {
      return false;  // nothing (more) to process
    }
    hnd.resume();        // RESUME (blocks until suspended again or the end)
    return !hnd.done();  // are we done?
  }

 private:
  CoroHandle hnd;
};

// The required promise class.
// It holds the coroutine user context.
struct CoroSimple::promise_type {
  // Required method to create handler
  CoroSimple get_return_object() {
    std::cout << "promise_type\tget_return_object\n";
    // returning object created from a static factory method is
    //  the most popular implementation.
    return CoroSimple{CoroHandle::from_promise(*this)};
  }

  // Required method to deal if the coroutine
  //  will start eagerly or will suspend (lazy start)
  std::suspend_always initial_suspend() const noexcept {
    std::cout << "promise_type\tinitial_suspend\n";
    return {};
  }

  // Required method to deal if the coroutine
  //  will return the context to the caller or will be destroyed
  std::suspend_always final_suspend() const noexcept {
    std::cout << "promise_type\tfinal_suspend\n";
    return {};
  }

  // Required method to deal with case of unhandled exception
  void unhandled_exception() { std::terminate(); }

  // Required method called on co_return with no value
  // If there is co_return with a value a method
  // return_value(T) is required, where T is the type returned
  // It is not allowed to have both methods!
  void return_void() const noexcept {
    std::cout << "promise_type\treturn_void\n";
  }
};

CoroSimple printSomeNumbers(int num) {
  std::cout << "Coroutine started with param: " << num << "\n";
  for (int val = 1; val <= num; ++val) {
    std::cout << " value: " << val << '\n';
    co_await std::suspend_always{};  // Suspend execution here
  }
  std::cout << "\nCoroutine with param: " << num << " ended\n";
}

int main() {
  {
    std::cout << "main\t\tStarted\n";
    CoroSimple task = printSomeNumbers(3);
    std::cout << "\nmain\t\tTask created\n";

    while (task.resume()) {
      std::cout << "main\t\tin the loop\n";
    }

    std::cout << "\nmain\t\tLeave inner block\n";
  }
  std::cout << "main\t\tEnded";
  return 0;
}
