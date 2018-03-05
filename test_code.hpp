#ifndef TEST_CODE_HPP
#define TEST_TXNS_HPP

#include "unit_tests.hpp"
#include "code.cpp"
#include <iostream>
using namespace blockchain;

void testCode(int& testNum,int testWanted) {
  #define cmd(x) (unsigned int)Opcodes::x
  CodeMemory c0({cmd(ADD),6,1, cmd(SPEND),6, cmd(QUIT), 5});
  auto x = c0.run(10);
  test(x.moneySpent.size() == 1 && x.moneySpent[0].getAmt() == 6 && x.gasUsed == 3,"Add and spend code, more than enough gas");
  c0 = CodeMemory({cmd(ADD),6,1, cmd(SPEND),6, cmd(QUIT), 5});
  x = c0.run(2);
  test(x.moneySpent.size() == 0 && x.gasUsed == 1,"Add and spend code, not enough gas");
  c0 = CodeMemory({cmd(SUB),6,1, cmd(SPEND),6, cmd(QUIT), 5});
  x = c0.run(3);
  test(x.moneySpent.size() == 1 && x.moneySpent[0].getAmt() == 4 && x.gasUsed == 3,"Sub and spend code, just enough gas");
  c0 = CodeMemory({cmd(MULT),6,2, cmd(SPEND),6, cmd(QUIT), 5});
  x = c0.run(10);
  test(x.moneySpent.size() == 1 && x.moneySpent[0].getAmt() == 10 && x.gasUsed == 5,"Mult and spend code");
  c0 = CodeMemory({cmd(DIV),6,5, cmd(SPEND),6, cmd(QUIT), 5});
  x = c0.run(10);
  test(x.moneySpent.size() == 1 && x.moneySpent[0].getAmt() == 1 && x.gasUsed == 7,"Div and spend code");
  c0 = CodeMemory({cmd(SHIFTR),11,8, cmd(SPEND),11, cmd(SHIFTL),11,16, cmd(SPEND),11, cmd(QUIT), 0x100});
  x = c0.run(10);
  test(x.moneySpent.size() == 2 && x.moneySpent[0].getAmt() == 1 && x.moneySpent[1].getAmt() == 0x10000 && x.gasUsed == 6,"Bitshift-R, Bitshift-L and spend code");
  c0 = CodeMemory({cmd(INV),5, cmd(SPEND),5, cmd(QUIT), (unsigned int)-1});
  x = c0.run(10);
  test(x.moneySpent.size() == 1 && x.moneySpent[0].getAmt() == 1 && x.gasUsed == 3,"Inv and spend code");
}

#endif
