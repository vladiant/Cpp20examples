// localTime.cpp
// https://godbolt.org/z/4893zPT7s
// gcc 13.1

#include <chrono>
#include <format>
#include <iostream>

int main() {
  std::cout << '\n';

  using std::chrono::floor;

  std::cout << "UTC  time" << '\n';
  auto utcTime = std::chrono::system_clock::now();
  std::cout << "  " << utcTime << '\n';
  std::cout << std::format("  {:}\n", utcTime);
  std::cout << "  " << floor<std::chrono::seconds>(utcTime) << '\n';

  std::cout << '\n';

  std::cout << "Local time" << '\n';
  auto localTime =
      std::chrono::zoned_time(std::chrono::current_zone(), utcTime);

  std::cout << "  " << localTime << '\n';
  std::cout << std::format("  {:}\n", localTime);
  std::cout << "  " << floor<std::chrono::seconds>(localTime.get_local_time())
            << '\n';

  auto offset = localTime.get_info().offset;
  std::cout << "  UTC offset: " << offset << '\n';

  std::cout << '\n';
}
