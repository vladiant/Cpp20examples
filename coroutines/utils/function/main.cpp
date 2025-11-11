#include <coroutine>
#include <cstdio>
#include <stdexcept>
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

  ~owning_handle() {
    if (handle_ != nullptr) handle_.destroy();
  }

 private:
  std::coroutine_handle<promise_type> handle_;
};

template <typename T>
struct Function;

template <typename T>
struct FunctionPromise {
  using handle_t = std::coroutine_handle<FunctionPromise<T>>;
  // To read the result from the coroutine,
  // the caller needs to have access to it
  Function<T> get_return_object() {
    // Construct a coroutine handle from this promise
    return Function<T>{handle_t::from_promise(*this)};
  }

  std::suspend_never initial_suspend() noexcept { return {}; }
  // Because we need to give the caller a chance to read
  // the result, the coroutine now cannot finish after
  // completing the body. It needs to stick around.
  std::suspend_always final_suspend() noexcept { return {}; }

  // The mechanism used to "return" a result.
  // We can simply store it.
  template <std::convertible_to<T> Arg>
  void return_value(Arg&& result) {
    result_ = std::forward<Arg>(result);
  }

  void unhandled_exception() {
    // May be empty, may explicitly terminate
    // std::terminate();
    // or store exception for further processing
    exception_ = std::current_exception();
  }

  // The result of the coroutine stored in the promise
  T result_;
  std::exception_ptr exception_;
};

template <typename T>
struct Function {
  using promise_type = FunctionPromise<T>;

  // Store the coroutine handle
  explicit Function(promise_type::handle_t handle) : handle_(handle) {}

  // Interface for extracting the result
  // T get() { return handle_.promise().result_; }
  // operator T() { return get(); }
  operator T() {
    if (handle_.promise().exception_ != nullptr)
      std::rethrow_exception(handle_.promise().exception_);
    return handle_.promise().result_;
  }

 private:
  owning_handle<promise_type> handle_;
};

int main() {
  auto coro = [] -> Function<int> {
    puts("Running...");
    throw std::runtime_error("Error");
    co_return 42;
  };

  // auto x = coro();
  // int result = x.get(); // Get the result
  // printf("result == %d\n", result);

  // int y = coro(); // Same, but through implicit conversion
  // printf("y == %d\n", y);

  try {
    int x = coro();
    printf("x == %d\n", x);
  } catch (const std::exception& e) {
    printf("Exception caught: %s\n", e.what());
  }

  return 0;
}
