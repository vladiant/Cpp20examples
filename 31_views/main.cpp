#include <iostream>
#include <ranges>
#include <set>
#include <vector>

void print(const std::ranges::input_range auto& coll) {
  for (const auto& elem : coll) {
    std::cout << elem << ' ';
  }
  std::cout << '\n';
}

int main() {
  std::vector<int> col1{0, 8, 15, 47, 11, 42};
  std::set<int> col2{0, 8, 15, 47, 11, 42};

  print(col1);
  print(col2);

  print(std::views::take(col1, 3));
  print(std::views::take(col2, 3));

  print(col1 | std::views::take(3));
  print(col2 | std::views::take(3));

  print(col1 | std::views::take(3) |
        std::views::transform([](auto v) { return std::to_string(v) + 's'; }));
  print(col2 | std::views::take(3) |
        std::views::transform([](auto v) { return std::to_string(v) + 's'; }));

  return 0;
}
