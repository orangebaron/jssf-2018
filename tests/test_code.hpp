#ifndef TEST_CODE_HPP
#define TEST_TXNS_HPP

#include "unit_tests.hpp"
#include "../code.cpp"
#include <iostream>
using namespace blockchain;

#define cmd(x) (unsigned int)Opcodes::x
#define arr(...) { __VA_ARGS__ }
#define singleSpendCodeTest(arr,gasLim,spent,gas,name) \
c0 = CodeMemory(arr); \
x = c0.run(gasLim); \
test(x.moneySpent.size() == 1 && x.moneySpent[0].getAmt() == spent && x.gasUsed == gas,name);

void testCode(int& testNum,int testWanted) {
  CodeMemory c0({cmd(SHIFTR),11,8, cmd(SPEND),11, cmd(SHIFTL),11,16, cmd(SPEND),11, cmd(QUIT), 0x100});
  auto x = c0.run(10);
  test(x.moneySpent.size() == 2 && x.moneySpent[0].getAmt() == 1 && x.moneySpent[1].getAmt() == 0x10000 && x.gasUsed == 6,"Bitshift-R, Bitshift-L code");

  c0 = CodeMemory({cmd(ADD),6,1, cmd(SPEND),6, cmd(QUIT), 5});
  x = c0.run(2);
  test(x.moneySpent.size() == 0 && x.gasUsed == 1,"Add code, not enough gas");

  singleSpendCodeTest(arr(cmd(ADD),6,1, cmd(SPEND),6, cmd(QUIT), 5),10,6,3,"Add code, more than enough gas");
  singleSpendCodeTest(arr(cmd(SUB),6,1, cmd(SPEND),6, cmd(QUIT), 5),3,4,3,"Sub code, just enough gas");
  singleSpendCodeTest(arr(cmd(MULT),6,2, cmd(SPEND),6, cmd(QUIT), 5),10,10,5,"Mult code");
  singleSpendCodeTest(arr(cmd(DIV),6,5, cmd(SPEND),6, cmd(QUIT), 5),10,1,7,"Div code");
  singleSpendCodeTest(arr(cmd(INV),5, cmd(SPEND),5, cmd(QUIT), (unsigned int)-1),10,1,3,"Inv code");
  singleSpendCodeTest(arr(cmd(JMP),5, cmd(SPEND),5, cmd(QUIT), cmd(SPEND),10, cmd(QUIT)),10,1,3,"Jmp code");
}

#endif
