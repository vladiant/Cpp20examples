#include <algorithm>
#include <array>
#include <format>
#include <iostream>

void print(const auto& rg) {
  for (const auto& value : rg) {
    std::cout << value << '\n';
  }
}

struct Coord {
  double x, y, z;

  auto operator<=>(const Coord&) const = default;
  friend std::ostream& operator<<(std::ostream& out, const Coord& data) {
    out << std::format("{}/{}/{}", data.x, data.y, data.z);
    return out;
  }
};

int main() {
  std::array points{Coord{1, 2, 3}, Coord{4, 5, 6}, Coord{0, 2, 0},
                    Coord{0, 0, 2}};
  std::ranges::sort(points);
  print(points);
  return 0;
}
