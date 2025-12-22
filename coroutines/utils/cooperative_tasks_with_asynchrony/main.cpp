#include <cassert>
#include <chrono>
#include <coroutine>
#include <cstdio>
#include <format>
#include <list>
#include <queue>
#include <thread>
#include <utility>

struct Scheduler {
  using time_point = std::chrono::time_point<std::chrono::system_clock>;

  // Add a coroutine under the control of the scheduler
  void enqueue(std::coroutine_handle<> handle,
               time_point time = std::chrono::system_clock::now()) const {
    pending_coroutines_.push(std::make_pair(time, handle));
  }

  void run() const {
    while (not pending_coroutines_.empty()) {
      if (pending_coroutines_.top().first > std::chrono::system_clock::now()) {
        std::this_thread::sleep_until(pending_coroutines_.top().first);
      }

      auto active = pending_coroutines_.top().second;
      pending_coroutines_.pop();

      active.resume();

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
      // The argument is a handle to the suspended coroutine.
      Scheduler{}.enqueue(ctx, time_);
    }
    // Called as the last part of evaluating co_await,
    // the coroutine is resumed just before this call
    // (if it was suspended in the first place).
    void await_resume() {}

    time_point time_;
  };

  WakeupAwaitable wake_up() const {
    return WakeupAwaitable{std::chrono::system_clock::now()};
  }
  WakeupAwaitable wake_up(time_point time) const {
    return WakeupAwaitable{time};
  }

 private:
  // Monostate
  using timed_coroutine = std::pair<time_point, std::coroutine_handle<>>;
  static std::priority_queue<timed_coroutine, std::vector<timed_coroutine>,
                             std::greater<>>
      pending_coroutines_;
};

std::priority_queue<Scheduler::timed_coroutine,
                    std::vector<Scheduler::timed_coroutine>, std::greater<>>
    Scheduler::pending_coroutines_{};

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
    auto await_transform(
        std::chrono::time_point<std::chrono::system_clock> time) const {
      return Scheduler{}.wake_up(time);
    }
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
  using namespace std::chrono;
  auto coro = [] -> Task {
    auto time = system_clock::now();
    // printf("%ld\n", system_clock::to_time_t(system_clock::now()));
    std::puts(std::format("{}", system_clock::now()).c_str());
    co_await (time + 200ms);
    std::puts(std::format("{}", system_clock::now()).c_str());
    co_await (time + 400ms);
    std::puts(std::format("{}", system_clock::now()).c_str());
  };

  coro().detach();
  coro().detach();

  Scheduler{}.run();
}
