#include <iostream>
#include <list>
#include <ranges>
#include <vector>

void print(std::ranges::view auto coll) {
  for (const auto& elem : coll) {
    std::cout << elem << ' ';
  }
  std::cout << '\n';
}

int main() {
  std::vector<int> col1{0, 8, 15, 47, 11, 42};
  std::list<int> col2{0, 8, 15, 47, 11, 42};

  print(std::views::all(col1));
  print(std::views::all(col2));

  print(col1 | std::views::take(3));
  print(col2 | std::views::take(3));

  print(col1 | std::views::drop(3));

  print(col2 | std::views::drop(3));  // fails for print(const &
  for (int v : col2 | std::views::drop(3)) {
    std::cout << v << ' ';
  }
  std::cout << '\n';

  return 0;
}
