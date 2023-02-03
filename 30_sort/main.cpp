#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <vector>

template <std::ranges::range Coll, typename T>
void add(Coll& col, const T& val) requires std::convertible_to<
    T, std::ranges::range_value_t<Coll>> {
  if constexpr (requires { col.push_back(val); }) {
    col.push_back(val);
  } else {
    col.insert(val);
  }
}

int main() {
  std::vector<int> col1;
  std::set<int> col2;

  add(col1, 42);
  add(col2, 42);

  for (const auto& i : col1) {
    std::cout << i << ' ';
  }
  for (const auto& i : col2) {
    std::cout << i << ' ';
  }

  // std::sort(col1.begin(), col1.end()); // C++98
  std::ranges::sort(col1);
  // std::ranges::sort(col2); // note:   ‘std::random_access_iterator_tag’ is
  // not a base of ‘std::bidirectional_iterator_tag’

  return 0;
}
