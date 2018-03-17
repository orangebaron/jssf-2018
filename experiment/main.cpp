#include "experiment.cpp"
#include "../chain/code.cpp"
#include "../chain/txn.cpp"
#include "../chain/types.cpp"
#include <iostream>

void test(ChainType c) {
  //std::cout<<"m"<<std::endl;
  const int numMiners = 1;
  MinerList m;
  for (int i=0;i<numMiners;i++) m.push_back(new Miner(Expt2,m,i));
  std::this_thread::sleep_for(std::chrono::seconds(1));
  User u(m,1,c);
  std::this_thread::sleep_for(std::chrono::seconds(10));
  u.stop=true;
  m.clear();
  //std::cout<<u.totalTxnTime<<std::endl;
  //std::cout<<u.numSuccessfulTxns<<std::endl;
  std::cout<<((double)u.totalTxnTime/u.numSuccessfulTxns)<<std::endl;
}
int main() {
  test(Expt1);
  test(Expt2);
  test(Expt3);
  test(Expt4);
}
