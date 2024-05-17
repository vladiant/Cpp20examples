#include <chrono>
#include <iostream>

using namespace std::chrono;

int main() {
  // auto firstDay = 2021y / 1 / 31;
  auto firstDay = 2021y / 1 / std::chrono::last;

  auto lastDay = 12 / 31d / 2021;

  for (auto d = firstDay; d <= lastDay; d += std::chrono::months{1}) {
    std::cout << d.ok() << ": Party\n";
  }

  return 0;
}
