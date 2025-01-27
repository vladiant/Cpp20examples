// Copyright (c) Andreas Fertig.
// SPDX-License-Identifier: MIT
// https://github.com/andreasfertig/heise-2025-01-cpp20-coroutinen-teil-2

#include <coroutine>
#include <cstdio>
#include <exception>  // std::terminate
#include <iostream>
#include <iterator>
#include <new>
#include <utility>
#include <vector>

class Generator {
public:
  struct promise_type {
    int _val{};

    Generator get_return_object()
    {
      return Generator{*this};
    }
    std::suspend_never initial_suspend() noexcept
    {
      return {};
    }
    std::suspend_always final_suspend() noexcept
    {
      return {};
    }
    std::suspend_always yield_value(int v)
    {
      _val = v;
      return {};
    }

    void return_void() noexcept {}
    void unhandled_exception() noexcept {}
  };

private:
  // class Generator {
  std::coroutine_handle<promise_type> mHandle{};

public:
  explicit Generator(promise_type& p) noexcept
  : mHandle{std::coroutine_handle<
      promise_type>::from_promise(p)}
  {}

  Generator(Generator&& rhs) noexcept
  : mHandle{std::exchange(rhs.mHandle, nullptr)}
  {}

  ~Generator() noexcept
  {
    if(mHandle) { mHandle.destroy(); }
  }

  int value() const { return mHandle.promise()._val; }

  bool finished() const { return mHandle.done(); }

  void resume()
  {
    if(not finished()) { mHandle.resume(); }
  }

  struct iterator {
    std::coroutine_handle<promise_type>
      mHandle{};  // #A

    bool
    operator==(std::default_sentinel_t) const  // #B
    {
      return mHandle.done();
    }

    iterator& operator++()  // #C
    {
      mHandle.resume();
      return *this;
    }

    int operator*() const  // #D
    {
      return mHandle.promise()._val;
    }
  };

  // class Generator {
  // public:
  // ...
  iterator                begin() { return {mHandle}; }
  std::default_sentinel_t end() { return {}; }
  // };
};

Generator interleaved(std::vector<int> a,
                      std::vector<int> b)
{
  auto lamb =
    [](std::vector<int>& v) -> Generator {  // #A
    for(const auto& e : v) { co_yield e; }
  };

  auto x = lamb(a);  // #B
  auto y = lamb(b);

  // #C
  while(not x.finished() or not y.finished()) {
    if(not x.finished()) {  // #D
      co_yield x.value();
      x.resume();
    }

    if(not y.finished()) {  // #E
      co_yield y.value();
      y.resume();
    }
  }
}

void UseIterator()
{
  std::vector a{2, 4, 6, 8};
  std::vector b{3, 5, 7, 9};

  Generator g{interleaved(std::move(a), std::move(b))};

  for(const auto& e : g) { std::cout << e << '\n'; }
}

int main()
{
  std::vector a{2, 4, 6, 8};
  std::vector b{3, 5, 7, 9};

  Generator g{interleaved(std::move(a), std::move(b))};

  while(not g.finished()) {
    std::cout << g.value() << '\n';

    g.resume();
  }
}
