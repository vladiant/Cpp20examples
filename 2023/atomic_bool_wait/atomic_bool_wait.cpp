#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

std::vector<int> mySharedWork;
std::atomic<bool> atomicBool{false};

void waitingForWork() {
  std::cout << "Waiting " << '\n';
  atomicBool.wait(false);
  mySharedWork[1] = 2;
  std::cout << "Work done " << '\n';
}

void setDataReady() {
  mySharedWork = {1, 0, 3};
  std::cout << "Data prepared" << '\n';
  atomicBool.store(true);
  atomicBool.notify_one();
}

int main() {
  std::cout << '\n';

  std::thread t1(waitingForWork);
  std::thread t2(setDataReady);

  t1.join();
  t2.join();

  for (auto v : mySharedWork) {
    std::cout << v << " ";
  }

  std::cout << "\n\n";
}
