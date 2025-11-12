#include <cassert>
#include <coroutine>
#include <cstdio>
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

  ~owning_handle() {
    if (handle_ != nullptr) handle_.destroy();
  }

 private:
  std::coroutine_handle<promise_type> handle_;
};

template <typename T>
struct LazyFunction;

template <typename T>
struct LazyFunctionPromise {
  using handle_t = std::coroutine_handle<LazyFunctionPromise<T>>;
  // Get the caller access to the handle
  LazyFunction<T> get_return_object() {
    return LazyFunction<T>{handle_t::from_promise(*this)};
  }

  // Immediately return control to the caller
  std::suspend_always initial_suspend() noexcept { return {}; }
  // Keep the coroutine alive so we can read the result
  std::suspend_always final_suspend() noexcept { return {}; }

  // Support for co_return
  template <std::convertible_to<T> Arg>
  void return_value(Arg&& result) {
    result_ = std::forward<Arg>(result);
  }
  void unhandled_exception() {}
  T result_;
};

template <typename T>
struct LazyFunction {
  using promise_type = LazyFunctionPromise<T>;

  // Store the coroutine handle
  explicit LazyFunction(promise_type::handle_t handle) : handle_(handle) {}

  T get() {
    // If the coroutine body has not finished running, resume it
    if (not handle_.done()) handle_.resume();
    return handle_.promise().result_;
  }

  operator T() { return get(); }

 private:
  owning_handle<promise_type> handle_;
};

int main() {
  auto coro = [] -> LazyFunction<int> {
    std::puts("Running...");
    co_return 42;
  };

  {
    auto x = coro();
    int result = x.get();  // Run the coroutine and get the result
    int other = x.get();   // Get the result

    printf("result == %d, other == %d\n", result, other);
  }  // coroutine held by x destroyed

  {
    int y = coro();  // Same, but through implicit conversion,
                     // coroutine is run and destroyed as part
                     // of this expression.
    printf("y == %d\n", y);
  }
}
