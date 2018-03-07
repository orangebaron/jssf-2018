#ifndef CODE_HPP
#define CODE_HPP

#include "types.hpp"

namespace blockchain {
  struct RunOtp {
    vector<TxnOtp> moneySpent;
    vector<StorageChange> storageChanged;
    GasAmt gasUsed = 0;
  };
  class CodeMemory {
    size_t location = 0;
    vector<unsigned int> memory;
  public:
    CodeMemory(vector<unsigned int> memory): memory(memory) {}
    RunOtp run(GasAmt gasLimit, const ExtraChainData&, Pubkey);
  };
  enum class Opcodes : unsigned int {
    QUIT,
    ADD,SUB,MULT,DIV,SHIFTR,SHIFTL,INV,
    AND,OR,XOR,NAND,NOR,XNOR,
    JMP,JE,JNE,JL,JG,JLE,JGE,
    SPEND,
    MEMGET,MEMSET,
    MOV,SET,
    DEBUGPRINT
  };
}

#endif
