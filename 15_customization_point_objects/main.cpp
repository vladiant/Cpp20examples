#include <algorithm>
#include <iostream>
#include <ranges>
#include <vector>

template <typename T, typename... Args>
auto mergeValue(const T& rg, Args&&... vals) {
  // Create compile time vector
  std::vector<std::ranges::range_value_t<T>> v{std::ranges::begin(rg),
                                               std::ranges::end(rg)};

  (...,
   v.push_back(std::forward<decltype(vals)>(vals)));  // merge passed values

  std::ranges::sort(v);  // sort all elements

  // return extended collection
  return v;
}

int main() {
  // Compile time initialization of array
  constexpr int orig[]{0, 8, 15, 132, 4, 77, 3};

  // Compile time initialization of sorted extended array
  auto merged = mergeValue(orig, 42, 4);
  for (const auto& i : merged) {
    std::cout << i << ' ';
  }

  return 0;
}
