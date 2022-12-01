#include <vector>
#include <iostream>

template<typename Coll, typename T>
void add(Coll& col, const T& val)
{
  col.push_back(val);
}

int main() {
  std::vector<int> col;
  
  add(col, 42); 

  for (const auto& i : col) {
    std::cout << i << ' ';
  }

  return 0;
}
