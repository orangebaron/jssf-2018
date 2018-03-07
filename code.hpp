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
    STRGET,STRSET,
    MOV,SET,
    DEBUGPRINT
  };
  class ContractCreation: public Hashable, public Validable, public Applyable {
    CodeMemory mem;
    Pubkey key;
  public:
    ContractCreation(CodeMemory,Pubkey);
    Pubkey getKey();
    virtual Hash getHash() const;
    virtual bool getValid(const ExtraChainData&,ValidsChecked&) const;
    virtual void apply(ExtraChainData&) const;
    virtual void unapply(ExtraChainData&) const;
  };
}

#endif
