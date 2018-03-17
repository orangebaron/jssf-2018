#include "experiment.cpp"
#include "../chain/code.cpp"
#include "../chain/txn.cpp"
#include "../chain/types.cpp"
#include <iostream>

void test(ChainType c,int fakes=0) {
  MinerList m;
  for (int i=0;i<10;i++) m.push_back(new Miner(Expt2,m,i));
  std::this_thread::sleep_for(std::chrono::seconds(1));
  User u(m,10,c,fakes);
  std::this_thread::sleep_for(std::chrono::seconds(10));
  u.stop=true;
  int totFees = 0;
  for (auto miner: m) totFees+=miner->totFees;
  m.clear();
  //std::cout<<u.totalTxnTime<<std::endl;
  //std::cout<<u.numSuccessfulTxns<<std::endl;
  std::cout<<((float)u.totalTxnTime/u.numSuccessfulTxns)<<'\t'<<totFees;
}
#define testThing(x) \
test(Expt##x ); \
std::cout<<'\t'; \
test(Expt##x ,10); \
std::cout<<std::endl;
int main() {
  for (int i=0;i<12;i++) {
    testThing(1);testThing(2);testThing(3);testThing(4);
    std::cout<<"------"<<std::endl;
  }
}
