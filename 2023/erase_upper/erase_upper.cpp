#include <iostream>
#include <string>

int main() {
  std::string str{"Only For TesTing PurPose."};
  std::cout << "str: " << str << '\n';

  std::erase_if(str, [](char c) { return std::isupper(c); });
  std::cout << "str: " << str << '\n';
}
