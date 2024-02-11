#include <deque>
#include <iostream>
#include <string>
#include <vector>

auto sumInt = [](int fir, int sec) { return fir + sec; };         // only ints
auto sumGen = [](auto fir, auto sec) { return fir + sec; };       // arbitrary types
auto sumTem = []<typename T>(T fir, T sec) { return fir + sec; }; // arbitrary, but identical types (C++20)


auto lambdaVector = []<typename T>(const std::vector<T>& vec) { return vec.size(); };

int main() {
   
    std::cout << '\n';
    
    std::vector vec{1, 2, 3, 4, 5};
    std::cout << "lambdaVector(vec): " << lambdaVector(vec) << '\n';
    
    std::deque deq{1, 2, 3, 4, 5};
    // std::cout << "lambdaVector(deq): " << lambdaVector(deq) << '\n'; ERROR
    
}

