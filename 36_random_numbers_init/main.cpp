#include <iostream>
#include <random>
#include <vector>

template <typename T>
void randomInitOld(T& values) {
  std::random_device rd;
  std::default_random_engine dre{rd()};
  std::uniform_int_distribution<int> random{1, 10};

  for (auto& v : values) {
    v += random(dre);
  }
}

void randomInit(std::ranges::forward_range auto& values) requires std::integral<
    std::ranges::range_value_t<decltype(values)>> {
  std::random_device rd;
  std::default_random_engine dre{rd()};
  std::uniform_int_distribution<int> random{1, 10};

  for (auto& v : values) {
    v += random(dre);
  }
}

void print(const std::ranges::input_range auto& coll) {
  for (const auto& elem : coll) {
    std::cout << elem << ' ';
  }
  std::cout << '\n';
}

int main() {
  std::vector col1{0, 0, 0, 0, 0, 0, 0, 0};

  randomInit(col1);
  print(col1);

  return 0;
}
