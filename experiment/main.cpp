#include "experiment.cpp"
#include "../chain/code.cpp"
#include "../chain/txn.cpp"
#include "../chain/types.cpp"
#include <iostream>

int main() {
  std::cout<<"m"<<std::endl;
  const int numMiners = 3;
  MinerList m;
  for (int i=0;i<numMiners;i++) m.push_back(new Miner(Expt1,m,i));
  std::this_thread::sleep_for(std::chrono::seconds(1));
  User u(m,1,Expt1);
  auto t = std::chrono::system_clock::now();
  while (std::chrono::system_clock::now()-t<std::chrono::seconds(60)) {
    for (auto miner: m) while (miner->deletableThreads.size()>0) {
      miner->deletableThreads[0]->join();
      miner->deletableThreads.erase(miner->deletableThreads.begin());
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  u.stop=true;
  m.clear();
  return 0;
}
