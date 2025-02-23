// Copyright (c) Andreas Fertig.
// SPDX-License-Identifier: MIT
// https://github.com/andreasfertig/heise-2025-02-cpp20-coroutinen-teil-4

#include <cassert>
#include <coroutine>
#include <cstdio>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <version>

using std::byte;
std::byte operator""_B(char c) { return static_cast<byte>(c); }
std::byte operator""_B(unsigned long long c) { return static_cast<byte>(c); }

struct Generator {
  struct promise_type {
    std::optional<std::string> mValue{};
    std::byte mLastByte{};

    Generator get_return_object() { return Generator{this}; }
    auto yield_value(std::string value) noexcept {
      mValue = std::move(value);
      return std::suspend_always{};
    }

    [[nodiscard]] auto await_transform(std::byte) {
      struct awaiter {
        std::byte& mRecentByte;
        constexpr bool await_ready() const noexcept { return false; }
        void await_suspend(std::coroutine_handle<>) const noexcept {}
        std::byte await_resume() { return mRecentByte; }
      };

      return awaiter{mLastByte};
    }

    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() noexcept {}
    void unhandled_exception() noexcept {}
  };

  using Handle = std::coroutine_handle<promise_type>;
  Handle mCoroHdl{};

  std::optional<std::string> operator()() {
    return std::exchange(mCoroHdl.promise().mValue, std::nullopt);
  }

  void SendData(std::byte b) {
    mCoroHdl.promise().mLastByte = b;
    if (not mCoroHdl.done()) {
      mCoroHdl.resume();
    }
  }

  explicit Generator(promise_type* p) noexcept
      : mCoroHdl(Handle::from_promise(*p)) {}

  Generator(Generator&& rhs) noexcept
      : mCoroHdl{std::exchange(rhs.mCoroHdl, nullptr)} {}

  ~Generator() noexcept {
    if (mCoroHdl) {
      mCoroHdl.destroy();
    }
  }
};

static const byte ESC{'H'};
static const byte SOF{0x10};

Generator Parse() {
  while (true) {
    byte b = co_await byte{};

    if (ESC != b) {
      continue;
    }

    // #A not looking at a start sequence
    if (b = co_await byte{}; SOF != b) {
      continue;
    }

    std::string frame{};
    while (true) {  // #B capture the full frame
      b = co_await byte{};

      if (ESC == b) {
        // #C skip this byte and look at the next  one
        b = co_await byte{};

        if (SOF == b) {
          co_yield frame;
          break;
        } else if (ESC != b) {
          break;  // #D out of sync
        }
      }

      frame += static_cast<char>(b);
    }
  }
}

void HandleFrame(const std::string& frame);

void ProcessStream(std::vector<byte>& stream, Generator& parse) {
  for (const auto& b : stream) {
    // #A Send the new byte to the waiting  Parse
    // coroutine
    parse.SendData(b);

    // #B Check whether we have a complete frame
    if (const auto& res = parse(); res.has_value()) {
      HandleFrame(res.value());
    }
  }
}

void HandleFrame(const std::string& frame) { printf("%s\n", frame.c_str()); }

int main() {
  std::vector<byte> fakeBytes1{0x70_B, ESC,   SOF, ESC, 'H'_B, 'e'_B, 'l'_B,
                               'l'_B,  'o'_B, ESC, SOF, 0x7_B, ESC,   SOF};

  // #C Create the Parse coroutine  and store the
  // handle in p
  auto p = Parse();

  // #D Process the bytes
  ProcessStream(fakeBytes1, p);

  // #E Simulate the reopening of the network  stream
  std::vector<byte> fakeBytes2{'W'_B, 'o'_B, 'r'_B, 'l'_B,
                               'd'_B, ESC,   SOF,   0x99_B};

  // #F We still use the former p  and feed it with new
  // bytes
  ProcessStream(fakeBytes2, p);
}
