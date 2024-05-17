#include <iostream>
#include <set>
#include <string>
#include <vector>

template <std::ranges::range Coll, typename T>
void add(Coll& col, const T& val) requires
    std::convertible_to<T, std::ranges::range_value_t<Coll>> {
  if constexpr (requires { col.push_back(val); }) {
    col.push_back(val);
  } else {
    col.insert(val);
  }
}

int main() {
  std::vector<int> col1;
  std::set<std::string> col2;

  add(col1, 42);
  add(col2, "ttt");

  for (const auto& i : col1) {
    std::cout << i << ' ';
  }
  for (const auto& i : col2) {
    std::cout << i << ' ';
  }

  return 0;
}
