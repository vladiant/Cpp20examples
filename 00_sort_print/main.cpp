#include <algorithm>
#include <array>
#include <iostream>

template <typename T>
void print(const T& rg) {
  for (const auto& value : rg) {
    std::cout << value << '\n';
  }
}

struct Coord {
  double x, y, z;
  bool operator<(const Coord& other) const {
    if (x != other.x) {
      return x < other.x;
    }

    if (y != other.y) {
      return y < other.y;
    }

    if (z != other.z) {
      return z < other.z;
    }

    return false;
  }
};

std::ostream& operator<<(std::ostream& out, const Coord& data) {
  out << "{" << data.x << "," << data.y << "," << data.z << "}";
  return out;
}

int main() {
  std::array points{Coord{1, 2, 3}, Coord{4, 5, 6}, Coord{0, 2, 0},
                    Coord{0, 0, 2}};
  std::sort(points.begin(), points.end());
  print(points);
  return 0;
}
