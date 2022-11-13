#include <coroutine>
#include <iostream>

// Compiles with: GCC 11.1 (GCC 10.2 -fcoroutines), Clang 15.0.0, msvc 19.30

class [[nodiscard]] CoGenerator {
 public:
  struct promise_type;
  using CoroHandle = std::coroutine_handle<promise_type>;

 public:
  CoGenerator(CoroHandle h) : hnd{h} {}
  ~CoGenerator() {
    if (hnd) {
      hnd.destroy();
    }
  }

  CoGenerator(const CoGenerator&) = delete;
  CoGenerator& operator=(const CoGenerator&) = delete;

 public:
  // A public method to get next number
  // Here we hide the mechanism of resuming the sleeping coroutine
  unsigned long long getNextPrime();

 private:
  CoroHandle hnd;
};

struct CoGenerator::promise_type {
  CoGenerator get_return_object() { return {CoroHandle::from_promise(*this)}; }

  std::suspend_always initial_suspend() const noexcept { return {}; }

  std::suspend_never final_suspend() const noexcept { return {}; }

  // Required method to call on co_yield.
  // The parameter type should correspond to the yielded value.
  std::suspend_always yield_value(unsigned long long num) noexcept {
    last = num;  // Store the computed prime for later use
    return {};   // and suspend the coroutine (return control to the caller)
  }

  void unhandled_exception() { std::terminate(); }

  void return_void() const noexcept {}

  unsigned long long last = 0;
};

unsigned long long CoGenerator::getNextPrime() {
  if (!hnd || hnd.done()) {
    std::cerr << "Invalid CoRo state!\n";
    return -1;
  }
  hnd.resume();
  return hnd.promise().last;
}

bool isPrime(unsigned long long num) {
  if (num != 2 && (num < 1 || num % 2 == 0)) return false;

  for (unsigned long long d = 3; d * d <= num; d += 2)
    if (num % d == 0) return false;
  return true;
}

CoGenerator allPrimeNumbers() {
  co_yield 2;

  unsigned long long num = 3;
  for (;;) {
    if (isPrime(num)) {
      co_yield num;
    }
    num += 2;
  }
}

int main() {
  int cnt = 10;
  CoGenerator gen = allPrimeNumbers();

  for (int i = 0; i < cnt; ++i) {
    std::cout << gen.getNextPrime() << ' ';
  }

  return 0;
}
