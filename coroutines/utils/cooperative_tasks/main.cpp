#include <cassert>
#include <coroutine>
#include <cstdio>
#include <list>
#include <utility>

struct Scheduler {
  // Add a coroutine under the control of the scheduler
  void enqueue(std::coroutine_handle<> handle) const {
    coroutines_.push_back(handle);
  }

  void run() const {
    // As long as we have active coroutines, continue running
    while (not coroutines_.empty()) {
      // Grab the first coroutine from the queue
      auto active = coroutines_.front();
      coroutines_.pop_front();

      active.resume();
      // The coroutine is owned by the scheduler,
      // meaning it is responsible for destroying it
      if (active.done()) active.destroy();
    }
  }

  struct WakeupAwaitable {
    // Opportunity for early continuation, returning true
    // will let the coroutine continue without suspending.
    bool await_ready() { return false; }
    // Called after the coroutine is suspended, controls
    // what happens next. The void returning variant returns
    // control to the caller.
    void await_suspend(std::coroutine_handle<> ctx) {
      // Re-schedule the suspended coroutine
      // ctx is a handle to the suspended coroutine
      Scheduler{}.enqueue(ctx);
    }
    // Called as the last part of evaluating co_await,
    // the coroutine is resumed just before this call
    // (if it was suspended in the first place).
    void await_resume() {}
  };

  WakeupAwaitable wake_up() const { return WakeupAwaitable{}; }

 private:
  // Monostate
  static std::list<std::coroutine_handle<>> coroutines_;
};

std::list<std::coroutine_handle<>> Scheduler::coroutines_{};

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

  std::coroutine_handle<> detach() { return std::exchange(handle_, {}); }

  ~owning_handle() {
    if (handle_ != nullptr) handle_.destroy();
  }

 private:
  std::coroutine_handle<promise_type> handle_;
};

struct Task {
  struct promise_type {
    using handle_t = std::coroutine_handle<promise_type>;
    // Get the caller access to the handle
    Task get_return_object() { return Task{handle_t::from_promise(*this)}; }
    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() {}
  };

  void detach() {
    // Give control of this coroutine to the scheduler
    Scheduler{}.enqueue(handle_.detach());
  }

  // Store the coroutine handle
  explicit Task(promise_type::handle_t handle) : handle_(handle) {}

 private:
  owning_handle<promise_type> handle_;
};

int main() {
  auto coro = [] -> Task {
    puts("stage one");
    co_await Scheduler{}.wake_up();
    puts("stage two");
    co_await Scheduler{}.wake_up();
    puts("stage three");
  };

  coro().detach();
  coro().detach();
  coro().detach();

  Scheduler{}.run();
}
