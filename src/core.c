#include "common.h"
#include "modules.h"
#include "memory.h"

#include <stdio.h>
#include <time.h>

static void core_print(struct hs_State* H, UNUSED s32 argCount) {
  for (s32 i = 1; i <= argCount; i++) {
    size_t length;
    const char* str = hs_toString(H, i, &length);
    fwrite(str, sizeof(char), length, stdout);
    if (i != argCount) {
      putc('\t', stdout);
    }
  }
  putc('\n', stdout);
  hs_pushNil(H);
}

static void core_input(struct hs_State* H, UNUSED s32 argCount) {
  s32 capacity = 8;
  s32 count = 0;
  char* input = ALLOCATE(H, char, capacity);

  s32 c = EOF;
  while ((c = getchar()) != '\n' && c != EOF) {
    if (count + 1 > capacity) {
      s32 oldCapacity = capacity;
      capacity = GROW_CAPACITY(capacity);
      input = GROW_ARRAY(H, char, input, oldCapacity, capacity);
    }
    input[count++] = (char)c;
  }

  hs_pushString(H, input, count);
  FREE_ARRAY(H, char, input, capacity);
}

static void core_toString(struct hs_State* H, UNUSED s32 argCount) {
  size_t length;
  const char* str = hs_toString(H, 1, &length);
  hs_pushString(H, str, length);
}

static void core_clock(struct hs_State* H, UNUSED s32 argCount) {
  hs_pushNumber(H, (f64)clock() / CLOCKS_PER_SEC);
}

struct hs_FuncInfo core[] = {
  {core_print, "print", -1},
  {core_toString, "toString", 1},
  {core_clock, "clock", 0},
  {core_input, "input", 0},
  {NULL, NULL, -1},
};

void openCore(struct hs_State* H) {
  hs_registerGlobalFunctions(H, core);
}
