#ifndef CODE_CPP
#define CODE_CPP

#include "code.hpp"
using namespace blockchain;

RunOtp CodeMemory::run() {
  RunOtp returnVal;
  auto m = memory.data();
  for (size_t codeLoc = 0, argsSize = 0; codeLoc < memory.size(); codeLoc += 1 + argsSize, argsSize = 0) {
    switch(memory[codeLoc]) {
    case 0: argsSize = 2; //ADD
      m[codeLoc+1+m[codeLoc+1]] += m[codeLoc+2];
      returnVal.gasUsed += 1;
      break;
    case 1: argsSize = 1;
      returnVal.moneySpent.push_back(TxnOtp(Pubkey(),m[codeLoc+1+m[codeLoc+1]]));
      returnVal.gasUsed += 2;
      break;
    }
  }
  return returnVal;
}

#endif
