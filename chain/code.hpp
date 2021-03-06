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
    CodeMemory();
    CodeMemory(const vector<unsigned int> memory);
    CodeMemory(const CodeMemory&);
    RunOtp run(const ExtraChainData& e, const ContractCall& caller) const;
    size_t getMemSize() const;
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
  class ContractCreation: public Hashable, public Validable, public Applyable, public WorkRequired {
    CodeMemory mem;
    Pubkey key;
  public:
    ContractCreation(CodeMemory,Pubkey);
    Pubkey getKey();
    virtual Hash getHash() const;
    virtual bool getValid(const ExtraChainData&,ValidsChecked&) const;
    virtual void apply(ExtraChainData&) const;
    virtual void unapply(ExtraChainData&) const;
    virtual WorkType getWork(WorkCalculated&) const;
  };
  class ContractCall: public Hashable, public Validable, public WorkRequired {
    Pubkey caller;
    Pubkey called;
    vector<unsigned int> args;
    TxnAmt amt;
    GasAmt maxGas;
    unsigned long id;
  public:
    ContractCall(Pubkey,Pubkey,vector<unsigned int>,TxnAmt,GasAmt,unsigned long);
    virtual Hash getHash() const;
    virtual bool getValid(const ExtraChainData&,ValidsChecked&) const;
    virtual WorkType getWork(WorkCalculated&) const;
    Pubkey getCaller() const;
    Pubkey getCalled() const;
    vector<unsigned int> getArgs() const;
    TxnAmt getAmt() const;
    GasAmt getMaxGas() const;
    unsigned long getId() const;
    RunOtp getOtp(const ExtraChainData&);
  };
  struct PunRwdTxn {
    PunRwdTxn(ContractCall& appliedTo,RunOtp result);
    ContractCall& appliedTo;
    RunOtp result;
  };
}

#endif
