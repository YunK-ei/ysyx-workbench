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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>


enum {
  TK_NOTYPE = 256, TK_EQ, TK_PLUS, TK_MIN, TK_MUL, TK_NUM, TK_LPA, TK_RPA,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", TK_PLUS},         // plus
  {"==", TK_EQ},        // equal
  {"\\-", TK_MIN},
  {"\\*", TK_MUL},
  {"[0-9]+", TK_NUM},
  {"\\(", TK_LPA},
  {"\\)", TK_RPA},

};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NUM: 
            tokens[nr_token].type = rules[i].token_type;
            assert(substr_len < 32);
            for(int j = 0; j < substr_len; j++)
              tokens[nr_token].str[j] = e[position - substr_len + j];
            break;
          default: tokens[nr_token].type = rules[i].token_type;
        }
        
        nr_token += 1;
        assert(nr_token < 32);
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses() { 
  int top_p = 0;
  for(int i = 0; i < nr_token; i++){
    if(top_p < 0)
      return false;
    if(tokens[i].type == TK_LPA)
      top_p += 1; 
    else
      if(tokens[i].type == TK_RPA)
        top_p -= 1;
      else
        continue;
  }
  Log("top_p at %d", top_p);
  if(!top_p) return true;
  else return false;
}

static int* opt_stack; 
static int* opr_stack; 
static int top_t; 
static int top_r;

void eval_once(){
  switch(opt_stack[top_t--]){
    case TK_PLUS:
      opr_stack[top_r - 1] = opr_stack[top_r] + opr_stack[top_r - 1];
      top_r -= 1;
      break;
    case TK_MIN:
      opr_stack[top_r - 1] = opr_stack[top_r] - opr_stack[top_r - 1];
      top_r -= 1;
      break;
    case TK_MUL:
      opr_stack[top_r - 1] = opr_stack[top_r] * opr_stack[top_r - 1];
      top_r -= 1;
      break;
  }
  
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  /* TODO: Insert codes to evaluate the expression. */
  assert(check_parentheses());   
  opt_stack = (int*) calloc(32, sizeof(int));
  opr_stack = (int*) calloc(32, sizeof(int));
  top_r = 0;
  top_t = 0;
  for(int i = 0; i < nr_token; i++)
  {
    assert(top_t >= 0);
    switch(tokens[i].type){
      case TK_NUM:
        top_r += 1;
        opr_stack[top_r] = atoi(tokens[i].str);
        
        break;
      case TK_LPA:
        top_t += 1;
        opt_stack[top_t] = TK_LPA;
        
        break;
      case TK_RPA:
        while(opt_stack[top_t] != TK_LPA){
          eval_once();
          assert(top_t >=0 && top_r >= 0);
        }
        top_t -= 1;
        break;
      case TK_MUL:
        if(tokens[i+1].type == TK_LPA){
          top_t += 1;
          opt_stack[top_t] = TK_MUL;
        }
        else{
          opr_stack[top_r] = opr_stack[top_r] * atoi(tokens[++i].str);
        }
        
        break;
      case TK_PLUS:
        while(opt_stack[top_t] == TK_MUL){
          eval_once();
          assert(top_t >= 0 && top_r >= 0);
        }
        top_t += 1;
        opt_stack[top_t] = TK_PLUS;
        break;
      case TK_MIN:
        while(opt_stack[top_t] == TK_MUL)
          eval_once();
        top_t += 1;
        opt_stack[top_t] = TK_MIN;
        break;
    }
  
 
  }
  while(top_t != 0){
    eval_once();
    assert(top_t >= 0 && top_r >= 0);
  }
  printf("val = %d\n", opr_stack[1]);
  free(opt_stack);
  free(opr_stack);
  return 0;
}

