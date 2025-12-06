#include <cassert>
#include <coroutine>
#include <cstddef>
#include <cstdio>
#include <iterator>
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

  std::coroutine_handle<promise_type> raw_handle() const { return handle_; }

  ~owning_handle() {
    if (handle_ != nullptr) handle_.destroy();
  }

 private:
  std::coroutine_handle<promise_type> handle_;
};

template <typename T>
struct Generator;

template <typename T>
struct GeneratorPromise {
  using handle_t = std::coroutine_handle<GeneratorPromise<T>>;
  // Get the caller access to the handle
  Generator<T> get_return_object() {
    return Generator<T>{handle_t::from_promise(*this)};
  }

  // Immediately return control to the caller
  std::suspend_always initial_suspend() noexcept { return {}; }
  // Keep the coroutine alive so we can read the final result
  std::suspend_always final_suspend() noexcept { return {}; }

  // Support for co_yield
  template <std::convertible_to<T> Arg>
  std::suspend_always yield_value(Arg&& result) {
    result_ = std::forward<Arg>(result);
    return {};
  }
  // Support for co_return
  void return_void() {}
  void unhandled_exception() {}
  T result_;
};

template <typename T>
struct Generator {
  using promise_type = GeneratorPromise<T>;

  // Store the coroutine handle
  explicit Generator(promise_type::handle_t handle) : handle_(handle) {}

  struct iterator {
    using value_type = T;
    using difference_type = ptrdiff_t;

    // Move-only interface
    iterator(owning_handle<promise_type> handle) : handle_(std::move(handle)) {}
    iterator(iterator&& other) noexcept
        : handle_{std::exchange(other.handle_, {})} {};
    iterator& operator=(iterator&& other) noexcept {
      handle_ = std::exchange(other.handle_, {});
      return *this;
    }

    // Read current value
    T& operator*() const { return handle_.promise().result_; }
    // Advance the iterator
    iterator& operator++() {
      assert(not handle_.done());
      handle_.resume();
      return *this;
    }
    void operator++(int) { return ++*this; }

    // We are done when the coroutine is done
    friend bool operator==(const iterator& i, std::default_sentinel_t) {
      return i.handle_.done();
    }

   private:
    owning_handle<promise_type> handle_;
  };

  iterator begin() {
    handle_.resume();
    return iterator{std::move(handle_)};
  }

  std::default_sentinel_t end() const noexcept { return {}; }

 private:
  owning_handle<promise_type> handle_;
};

int main() {
  auto coro = [] -> Generator<int> {
    co_yield 1;
    co_yield 2;
    co_yield 3;
    co_return;
  };

  auto empty = [] -> Generator<int> { co_return; };

  for (auto result : coro()) {
    printf("result == %d\n", result);
  }

  for (auto result : empty()) {
    printf("result == %d\n", result);
  }
}
