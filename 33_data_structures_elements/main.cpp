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

struct Coord {
  double x{0};
  double y{0};
  double z{0};

  auto operator<=>(const Coord&) const = default;

  friend std::ostream& operator<<(std::ostream& strm, const Coord& c) {
    return strm << '[' << c.x << '/' << c.y << '/' << c.z << ']';
  }
};

int main() {
  std::vector points{Coord{3, 2, 1}, Coord{1, 2, 3}, Coord{0, 0, 0},
                     Coord{4, 5, 6}};

  print(points);

  // std::ranges::subrange rg{points.begin(), EndValue<Coord{}>{}};
  // print(rg);

  return 0;
}
