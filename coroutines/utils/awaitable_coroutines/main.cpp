#include <cassert>
#include <chrono>
#include <coroutine>
#include <cstdio>
#include <list>
#include <queue>
#include <thread>
#include <utility>

template <typename promise_type>
struct owning_handle {
  owning_handle() : handle_() {}
  owning_handle(std::nullptr_t) : handle_(nullptr) {}
  owning_handle(std::coroutine_handle<promise_type> handle)
      : handle_(std::move(handle)) {}

  owning_handle(const owning_handle<promise_type>&) = delete;
  owning_handle(owning_handle<promise_type>&& other)
      : handle_(std::exchange(other.handle_, nullptr)) {}

  owning_handle<promise_type>& operator=(const owning_handle<promise_type>&) =
      delete;
  owning_handle<promise_type>& operator=(owning_handle<promise_type>&& other) {
    handle_ = std::exchange(other.handle_, nullptr);
    return *this;
  }

  promise_type& promise() const { return handle_.promise(); }

  bool done() const {
    assert(handle_ != nullptr);
    return handle_.done();
  }

  void resume() const {
    assert(handle_ != nullptr);
    return handle_.resume();
  }

  std::coroutine_handle<> raw_handle() const { return handle_; }

  ~owning_handle() {
    if (handle_ != nullptr) handle_.destroy();
  }

 private:
  std::coroutine_handle<promise_type> handle_;
};

struct ContinuationAwaitable {
  bool await_ready() noexcept { return false; }
  std::coroutine_handle<> await_suspend(std::coroutine_handle<>) noexcept {
    return continuation_;
  }
  void await_resume() noexcept {}
  std::coroutine_handle<> continuation_{std::noop_coroutine()};
};

template <typename T>
struct AwaitableFunction {
  struct promise_type {
    using handle_t = std::coroutine_handle<promise_type>;
    // Get the caller access to the handle
    AwaitableFunction get_return_object() {
      return AwaitableFunction{handle_t::from_promise(*this)};
    }

    std::suspend_always initial_suspend() noexcept { return {}; }
    ContinuationAwaitable final_suspend() noexcept { return {continuation_}; }
    // Same logic as in Function example
    template <std::convertible_to<T> Arg>
    void return_value(Arg&& result) {
      result_ = std::forward<Arg>(result);
    }
    void unhandled_exception() { std::terminate(); }

    std::coroutine_handle<> continuation_{std::noop_coroutine()};
    T result_;
  };

  // Store the coroutine handle
  explicit AwaitableFunction(std::coroutine_handle<promise_type> handle)
      : handle_(handle) {}

  // Awaitable interface
  bool await_ready() noexcept { return false; }
  // When we await on the result of calling a coroutine
  // the coroutine will start, but will also remember its caller
  std::coroutine_handle<> await_suspend(
      std::coroutine_handle<> caller) noexcept {
    // Remember the caller
    handle_.promise().continuation_ = caller;
    // Returning a coroutine handle will run the corresponding coroutine
    return handle_.raw_handle();
  }
  T await_resume() { return handle_.promise().result_; }

  void run_until_completion() {
    while (not handle_.done()) handle_.resume();
  }

 private:
  owning_handle<promise_type> handle_;
};

struct AwaitableTask {
  struct promise_type {
    using handle_t = std::coroutine_handle<promise_type>;
    // Get the caller access to the handle
    AwaitableTask get_return_object() {
      return AwaitableTask{handle_t::from_promise(*this)};
    }

    std::suspend_always initial_suspend() noexcept { return {}; }
    ContinuationAwaitable final_suspend() noexcept { return {continuation_}; }
    void return_void() {}
    void unhandled_exception() { std::terminate(); }

    std::coroutine_handle<> continuation_{std::noop_coroutine()};
  };

  // Store the coroutine handle
  explicit AwaitableTask(std::coroutine_handle<promise_type> handle)
      : handle_(handle) {}

  // Awaitable interface
  bool await_ready() noexcept { return false; }
  // When we await on the result of calling a coroutine
  // the coroutine will start, but will also remember its caller
  std::coroutine_handle<> await_suspend(
      std::coroutine_handle<> caller) noexcept {
    // Remember the caller
    handle_.promise().continuation_ = caller;
    // Returning a coroutine handle will run the corresponding coroutine
    return handle_.raw_handle();
  }
  void await_resume() {}

  void run_until_completion() {
    while (not handle_.done()) handle_.resume();
  }

 private:
  owning_handle<promise_type> handle_;
};

AwaitableTask child() { co_return; }

AwaitableTask parent() {
  // Start child and wait until it is completed
  co_await child();
  co_return;
}

AwaitableFunction<int> child_func() { co_return 42; }

AwaitableTask other_parent() {
  int v = co_await child_func();
  printf("Child returned: %d\n", v);
  co_return;
}

int main() {
  parent().run_until_completion();
  other_parent().run_until_completion();
}
