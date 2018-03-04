#ifndef CODE_HPP
#define CODE_HPP

#include "types.hpp"

namespace blockchain {
  struct RunOtp {
    vector<TxnOtp> moneySpent;
    GasAmt gasUsed = 0;
  };
  class CodeMemory {
    size_t location = 0;
    vector<unsigned int> memory;
  public:
    CodeMemory(vector<unsigned int> memory): memory(memory) {}
    RunOtp run(GasAmt gasLimit);
  };
}

#endif
