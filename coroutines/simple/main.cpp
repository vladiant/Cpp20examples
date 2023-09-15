// https://lucisqr.substack.com/p/reverse-engineering-c20-coroutines

#include <coroutine>
#include <iostream>

struct Y {
  bool await_ready() noexcept {
    puts("await_ready:1");
    return true;
  };
  void await_suspend(auto x) noexcept { puts("await_suspend:1"); };
  void await_resume() noexcept { puts("await_resume:1"); };
};

struct X {
  struct promise_type {
    Y initial_suspend() {
      puts("initial_suspend");
      return Y{};
    };
    Y final_suspend() noexcept {
      puts("final_suspend");
      return Y{};
    };
    X get_return_object() {
      puts("get_return_object");
      return X{};
    };
    void unhandled_exception(){};
  };
};

X doit() {
  puts("doit:1");
  for (int j = 0; j < 3; ++j) {
    puts("loop:1");
    co_await Y{};
    puts("loop:2");
  }
  puts("doit:2");
}

int main() {
  puts("main:1");
  auto j = doit();
  puts("main:2");
}