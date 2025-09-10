// https://godbolt.org/z/d7EPTGTdd
// How To Manage Asynchronous Control Flow With C++ Coroutines - Andreas Weis -
// ACCU 2025 https://www.youtube.com/watch?v=lKUVuaUbRDk
//
// * Spawn outermost coroutine and register scheduler
// * Call nested coroutines and propagate execution context
// * Suspend innermost coroutine
// * Resume innermost coroutine
// * Resume outer coroutine from inner coroutine

#include <cassert>
#include <coroutine>
#include <exception>
#include <optional>
#include <print>
#include <utility>

/* The scheduler will resume a suspended innermost coroutine with provided data.
 */
struct Scheduler {
  std::coroutine_handle<> to_resume;  // A nested coroutine to be resumed
  int resume_value = -1;              // Value that coroutine is waiting for

  /* Supply data to and resume the nested coroutine
   */
  void resumeWithData(int j) {
    resume_value = j;
    auto h = std::exchange(to_resume, std::coroutine_handle<>{});
    h.resume();
  }
};

/* Base class for all promises.
 */
struct promise_base {
  promise_base* parent = nullptr;  // link to parent in the async call stack
  promise_base* child = nullptr;   // link to child in the async call stack
  std::coroutine_handle<>
      parentHandle;  // handle to parent in the async call stack
  std::coroutine_handle<> selfHandle;  // handle to self
  Scheduler* scheduler = nullptr;      // pointer to a scheduler
  virtual ~promise_base() = default;

  /* Set a scheduler for this promise and all its descendants along the async
   * call stack
   */
  void setScheduler(Scheduler& s) {
    scheduler = &s;
    if (child) {
      child->setScheduler(s);
    }
  }
};

/* An awaitable that waits for data supplied by a scheduler.
 */
struct GetData {
  promise_base* promise = nullptr;
  bool await_ready() { return false; }
  template <typename Promise_T>
  void await_suspend(std::coroutine_handle<Promise_T> h) {
    promise = &(h.promise());
    // Depending on when the scheduler was set, it may have already propagated
    // to all nodes, or it may only reside in the root promise of the async call
    // stack. In the latter case, we retrieve it here and propagate it to all
    // nodes along the stack.
    // find scheduler
    Scheduler* sched = nullptr;
    for (promise_base* it = promise; it != nullptr; it = it->parent) {
      if (it->scheduler) {
        sched = it->scheduler;
        break;
      }
    }
    assert(sched);
    // propagate scheduler
    for (promise_base* it = promise; it != nullptr && !it->scheduler;
         it = it->parent) {
      it->scheduler = sched;
    }
    // Register ourselves as the coroutine that the scheduler should wake up
    assert(!sched->to_resume);
    sched->to_resume = h;
  }
  int await_resume() {
    // When we wake up, the scheduler will have the data that we were waiting
    // for
    return promise->scheduler->resume_value;
  }
};

/* Helper awaitable for retrieving the promise_base of the current coroutine
 */
struct GetCallStack {
  promise_base* ret;
  bool await_ready() { return false; }
  template <typename Promise_T>
  auto await_suspend(std::coroutine_handle<Promise_T> h) {
    ret = &(h.promise());
    return h;
  }
  promise_base* await_resume() { return ret; }
};

/* Async object
 */
template <typename T>
struct Async {
  struct promise_type : public promise_base {
    std::optional<T> opt_return_value = std::nullopt;

    Async get_return_object() {
      return Async{std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    std::suspend_always initial_suspend() {
      // We always suspend in the beginning; This way we ensure that
      // by the time the coroutine body starts executing, the links between
      // parent and child have already been established.
      return {};
    }
    auto final_suspend() noexcept {
      // When we suspend, we symmetrically transfer control to our parent
      // in the async call stack, if we have one.
      // Otherwise we just return to our caller on the normal call stack.
      struct ResumeCaller {
        std::coroutine_handle<> p;
        ResumeCaller(std::coroutine_handle<> h) noexcept : p(h) {}
        bool await_ready() noexcept { return false; }
        auto await_suspend(std::coroutine_handle<>) noexcept {
          return p ? p : std::noop_coroutine();
        }
        void await_resume() noexcept {}
      };
      return ResumeCaller{parentHandle};
    }
    void unhandled_exception() noexcept { std::terminate(); }
    void return_value(T v) {
      assert(!opt_return_value.has_value());
      opt_return_value = v;
    }
  };

  std::coroutine_handle<promise_type> self;

  Async(std::coroutine_handle<promise_type> h) : self(h) {}

  Async(Async&& rhs)
      : self(std::exchange(rhs.self, std::coroutine_handle<promise_type>{})) {}

  ~Async() {
    if (self) {
      self.destroy();
    }
  }

  bool await_ready() { return false; }
  template <typename OtherPromise_T>
  auto await_suspend(std::coroutine_handle<OtherPromise_T> h_other) {
    // Upon co_awaiting the task, populate the fields in promise_base
    // to establish the links between parent and child
    assert(self.promise().parent == nullptr);
    self.promise().parent = &(h_other.promise());
    self.promise().parentHandle = h_other;
    self.promise().selfHandle = self;
    assert(h_other.promise().child == nullptr);
    h_other.promise().child = &(self.promise());

    // Symmetrically transfer control back to the coroutine of the task,
    // so that the body of the coroutine actually starts executing
    return self;
  }
  T await_resume() {
    // Unregister ourselves from our parent when we resume
    self.promise().parent->child = nullptr;
    return self.promise().opt_return_value.value();
  }

  /* Set the scheduler for this async task
   * (and all its children on the async call stack that are already connected)
   */
  void setScheduler(Scheduler& s) { self.promise().setScheduler(s); }
};

/* Return type for the spawn_task function.
 * This really only retrieves the result of the operation to hand back to
 * the caller of spawn_task.
 */
template <typename T>
struct AsyncResult {
  struct promise_type : public promise_base {
    AsyncResult get_return_object() {
      return AsyncResult{
          std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    std::suspend_never initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void unhandled_exception() noexcept { std::terminate(); }
    void return_value(T val) { result = val; }
    std::optional<T> result;
  };

  std::coroutine_handle<promise_type> self;

  AsyncResult(std::coroutine_handle<promise_type> h) : self(h) {}

  AsyncResult(AsyncResult&& rhs)
      : self(std::exchange(rhs.self, std::coroutine_handle<promise_type>{})) {}

  ~AsyncResult() {
    if (self) {
      self.destroy();
    }
  }

  /* Retrieve the result of a completed asynchronous operation
   */
  T result() {
    assert(self && self.promise().result);
    return self.promise().result.value();
  }
};

/* Execute an Async task on a thread.
 * This symmetrically transfers control to the task, which could then suspend
 * again in its body somewhere. Once the task has run to completion, the result
 * can be obtained from the returned AsyncResult object.
 */
template <typename T>
AsyncResult<T> spawn_task(Scheduler& scheduler, Async<T> task) {
  task.setScheduler(scheduler);
  co_return co_await task;
}

// ===========================================================================
// ================================ User Code ================================
// ===========================================================================

Async<int> inner_function(int i) {
  std::println("In inner");
  // Note how we can suspend here without having to maintain a reference
  // to the scheduler in user code.
  int additional_data = co_await GetData{};
  // Helper object for observing the async call stack in the MSVC debugger;
  // At this point only inner_function() will be on the normal call stack,
  // its parents in the async call stack have already gone into suspension.
  auto callstack = co_await GetCallStack{};
  std::println("GetData returned {}", additional_data);
  co_return i + additional_data;
}

Async<int> inner_ready() { co_return 3; }

struct WrappedInt {
  int i;
};

Async<WrappedInt> middle_function(int i) {
  std::println("In middle");
  int const ii = co_await inner_function(i);
  std::println("inner returned {}", ii);
  co_return WrappedInt{.i = ii * co_await inner_ready()};
}

Async<int> outer_function(int i) {
  std::println("In outer");
  WrappedInt const res = co_await middle_function(i);
  std::println("middle returned {}", res.i);
  co_return res.i + 3;
}

int main() {
  std::println("*** Deciphering Coroutines 2 - Demo ***");
  Async<int> my_task = outer_function(11);
  // at this point nothing has happened really; we simply constructed the
  // coroutine frame for outer_function, but we didn't execute anything from
  // its body
  Scheduler sched;
  AsyncResult<int> async_result = spawn_task(sched, std::move(my_task));
  // at this point, all functions are suspended and inner_function is awaiting
  // the result that will be provided through the scheduler; if none of the
  // functions had needed to wait for such a result, all coroutines would have
  // run to completion at this point.
  sched.resumeWithData(2);
  // At this point, inner_function has been resumed by the scheduler and
  // all coroutines have run to completion; the result value can now be
  // obtained from AsyncResult
  std::println("Result value is {}", async_result.result());
}
