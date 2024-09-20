// timeOfDay.cpp
// https://godbolt.org/z/T7bGvsrY9
// gcc 13.1

#include <chrono>
#include <format>
#include <iostream>

int main() {
  using namespace std::chrono_literals;

  std::cout << std::boolalpha << '\n';

  constexpr auto timeOfDay =
      std::chrono::hh_mm_ss(10.5h + 98min + 2020s + 0.5s);

  std::cout << "timeOfDay: " << timeOfDay << '\n';
  std::cout << "timeOfDay" << std::format(": {:}", timeOfDay);

  std::cout << "\n\n";

  std::cout << "timeOfDay.hours(): " << timeOfDay.hours() << '\n';
  std::cout << "timeOfDay.minutes(): " << timeOfDay.minutes() << '\n';
  std::cout << "timeOfDay.seconds(): " << timeOfDay.seconds() << '\n';
  std::cout << "timeOfDay.subseconds(): " << timeOfDay.subseconds() << '\n';
  std::cout << "timeOfDay.to_duration(): " << timeOfDay.to_duration() << '\n';

  std::cout << '\n';

  std::cout << "std::chrono::hh_mm_ss(45700.5s): "
            << std::chrono::hh_mm_ss(45700.5s) << '\n';

  std::cout << '\n';
}
