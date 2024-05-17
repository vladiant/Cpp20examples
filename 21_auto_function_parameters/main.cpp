#include <iostream>
#include <vector>

void add(auto& col, const auto& val) { col.push_back(val); }

int main() {
  std::vector<int> col;

  add(col, 42);

  for (const auto& i : col) {
    std::cout << i << ' ';
  }

  return 0;
}
