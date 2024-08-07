/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6",
  "mcause", "mstatus", "mepc", "mtvec",
};

void isa_reg_display() {
        for(int i = 0; i < 32; ++i){
                printf("%-12s=  0x%-12lx", regs[i], (unsigned long int) cpu.gpr[i]);
                if(i % 4 == 3)
                  printf("\n");
        }

        for(int i = 0; i < 4; ++i){
          printf("%-12s=  0x%-12lx", regs[i+32], (unsigned long int) cpu.csr[i]);
          if(i % 4 == 3)
            printf("\n");
        }

        printf("%-12s=  0x%-12lx\n", "pc", (unsigned long int) cpu.pc);
}

word_t isa_reg_str2val(const char *s, bool *success) {
  int tmp = 0;
  while(tmp < 32)
  {
    if(strcmp(regs[tmp], s) == 0)
      return cpu.gpr[tmp];
    else
      tmp += 1;
  }

  while(tmp < 36)
  {
    if(strcmp(regs[tmp], s) == 0)
      return cpu.csr[tmp - 32];
    else
      tmp += 1;
  }

  if(strcmp("pc", s) == 0)
    return cpu.pc;
  else if (tmp == 32)
  {
    *success = false;
    return 0;
  }
  return 0;
}
