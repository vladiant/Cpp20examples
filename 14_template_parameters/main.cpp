#include <algorithm>
#include <array>
#include <iostream>
#include <ranges>
#include <vector>

template <typename T, typename... Args>
auto mergeValue(const T& rg, Args&&... vals) {
  // Create compile time vector
  using DataT = std::remove_cvref_t<decltype(rg)>::value_type;
  std::vector<DataT> v{rg.begin(), rg.end()};

  (...,
   v.push_back(std::forward<decltype(vals)>(vals)));  // merge passed values

  std::ranges::sort(v);  // sort all elements

  // return extended collection
  return v;
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
