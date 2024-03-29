#include <iostream>
#include <set>
#include <vector>

template <typename Coll>
concept HasPushBack = requires(Coll c, typename Coll::value_type v) {
  c.push_back(v);
};

void add(HasPushBack auto& col, const auto& val) { col.push_back(val); }

void add(auto& col, const auto& val) { col.insert(val); }

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

  return 0;
}
