// https://godbolt.org/z/5e5YTT4M5
// gcc 13.1
#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>

namespace chr = std::chrono;
using namespace std::literals;

void print(std::ranges::input_range auto&& rg) {
  for (const auto& day : rg) {
    std::cout << day << ":\n";
    if (chr::weekday{day} == chr::Monday) {
      std::cout << "I don't like Mondays\n";
    }
    // telco at noon local time
    auto tpTelco{chr::local_days{day} + 12h};
    chr::zoned_time telcoLocal{chr::current_zone(), tpTelco};
    for (auto tzName : {"Europe/Berlin", "America/Los_Angeles"}) {
      chr::zoned_time zt{tzName, telcoLocal};
      std::cout << std::format(" {:%D %R %Z}\n", zt);
    }
  }
}

int main() {
  std::array dates{10d / 11 / 2021, 2021y / 4 / 4, chr::November / 1 / 2021};
  std::ranges::sort(dates);
  print(dates);
  return 0;
}
