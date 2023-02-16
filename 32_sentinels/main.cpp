#include <iostream>
#include <ranges>
#include <vector>

void print(const std::ranges::input_range auto& coll) {
  for (const auto& elem : coll) {
    std::cout << elem << ' ';
  }
  std::cout << '\n';
}

template <auto Val>
struct EndValue {
  bool operator==(auto pos) const { return *pos == Val; }
};

int main() {
  std::vector<int> col1{0, 8, 15, 47, 11, -1, 13};

  print(col1);

  std::ranges::subrange rg{col1.begin() + 1, EndValue<-1>{}};
  print(rg);

  print(rg | std::views::take(3) |
        std::views::transform([](auto v) { return std::to_string(v) + 's'; }));

  return 0;
}
