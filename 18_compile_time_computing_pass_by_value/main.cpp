#include <algorithm>
#include <array>
#include <iostream>
#include <ranges>
#include <vector>

template <typename T>
consteval auto mergeValue(T rg, auto... vals) {
  // Create compile time vector
  std::vector<std::ranges::range_value_t<T>> v{std::ranges::begin(rg),
                                               std::ranges::end(rg)};

  (..., v.push_back(vals));  // merge passed values

  std::ranges::sort(v);  // sort all elements

  // return extended collection as array
  constexpr auto sz = std::ranges::ssize(rg) + sizeof...(vals);
  std::array<std::ranges::range_value_t<T>, sz> arr{};
  std::ranges::copy(v, arr.begin());
  return arr;
}

int main() {
  // Compile time initialization of array
  constexpr std::array orig{0, 8, 15, 132, 4, 77, 3};

  // Compile time initialization of sorted extended array
  auto merged = mergeValue(orig, 42, 4);
  for (const auto& i : merged) {
    std::cout << i << ' ';
  }

  return 0;
}
