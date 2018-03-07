#ifndef CODE_HPP
#define CODE_HPP

#include "types.hpp"

namespace blockchain {
  struct RunOtp {
    vector<TxnOtp> moneySpent;
    vector<StorageChange> storageChanged;
    GasAmt gasUsed = 0;
  };
  class ContractCall;
  class CodeMemory {
    size_t location = 0;
    vector<unsigned int> memory;
  public:
    CodeMemory(vector<unsigned int> memory);
    RunOtp run(const ExtraChainData& e, const ContractCall& caller);
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
  class ContractCall: public Hashable, public Validable {
    Pubkey caller;
    Pubkey called;
    vector<unsigned int> args;
    TxnAmt amt;
    GasAmt maxGas;
  public:
    ContractCall(Pubkey,Pubkey,vector<unsigned int>,TxnAmt,GasAmt);
    virtual Hash getHash() const;
    virtual bool getValid(const ExtraChainData&,ValidsChecked&) const;
    Pubkey getCaller() const;
    Pubkey getCalled() const;
    vector<unsigned int> getArgs() const;
    TxnAmt getAmount() const;
    GasAmt getMaxGas() const;
    RunOtp getOtp(const ExtraChainData&);
  };
}

#endif
