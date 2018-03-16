#include "experiment.cpp"
#include "../chain/code.cpp"
#include "../chain/txn.cpp"
#include "../chain/types.cpp"
#include <iostream>

int main() {
  std::cout<<"m"<<std::endl;
  const int numMiners = 60;
  MinerList m;
  for (int i=0;i<numMiners;i++) m.push_back(new Miner(Expt1,m,i));
  User u(m,1,Expt1);
  std::this_thread::sleep_for(std::chrono::seconds(60));
  m.clear();
  return 0;
}
