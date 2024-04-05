#include "state.h"

#include <stdlib.h>
#include <stdio.h>

#include "hobbylang.h"
#include "memory.h"
#include "compiler.h"
#include "object.h"
#include "table.h"
#include "tostring.h"
#include "modules.h"

void resetStack(struct hs_State* H) {
  H->stackTop = H->stack;
  H->frameCount = 0;
  H->openUpvalues = NULL;
}

static Value* getValueAt(struct hs_State* H, s32 index) {
  if (index >= 0) {
    Value* value = H->frames[H->frameCount - 1].slots + index + 1;
    return (value > H->stackTop) ? NULL : value;
  }

  Value* value = H->stackTop + index;
  return (value > H->stackTop) ? NULL : value;
}

void push(struct hs_State* H, Value value) {
  *H->stackTop = value;
  H->stackTop++;
}

Value pop(struct hs_State* H) {
  H->stackTop--;
  return *H->stackTop;
}

Value peek(struct hs_State* H, s32 distance) {
  return H->stackTop[-1 - distance];
}

struct hs_State* hs_newState() {
  struct hs_State* H = malloc(sizeof(struct hs_State));

  H->objects = NULL;
  H->parser = ALLOCATE(H, struct Parser, 1);
  H->bytesAllocated = 0;
  H->nextGc = 1024 * 1024;
  H->grayCount = 0;
  H->grayCapacity = 0;
  H->grayStack = NULL;
  resetStack(H);
  initTable(&H->strings);
  initTable(&H->globals);

  openCore(H);

  return H;
}

void hs_freeState(struct hs_State* H) {
  freeTable(H, &H->strings);
  freeTable(H, &H->globals);
  freeObjects(H);
  FREE(H, struct Parser, H->parser);

  free(H);
}

void hs_pop(struct hs_State* H) {
  pop(H);
}

void hs_setGlobal(struct hs_State* H, const char* name) {
  push(H, NEW_OBJ(copyString(H, name, strlen(name))));
  tableSet(H, &H->globals, AS_STRING(peek(H, 0)), peek(H, 1));
  pop(H); // name
  pop(H); // value
}

void hs_registerGlobalFunctions(struct hs_State* H, struct hs_FuncInfo* funcs) {
  for (struct hs_FuncInfo* info = funcs; info->func != NULL; info++) {
    hs_pushCFunction(H, info->func, info->argCount);
    hs_setGlobal(H, info->name);
  }
}

void hs_pushNil(struct hs_State* H) {
  push(H, NEW_NIL);
}

void hs_pushNumber(struct hs_State* H, double v) {
  push(H, NEW_NUMBER(v));
}

void hs_pushBoolean(struct hs_State* H, bool v) {
  push(H, NEW_BOOL(v));
}

void hs_pushString(struct hs_State* H, const char* str, size_t length) {
  push(H, NEW_OBJ(copyString(H, str, length)));
}

void hs_pushOwnedString(struct hs_State* H, char* str, size_t length) {
  push(H, NEW_OBJ(takeString(H, str, length)));
}

void hs_pushCFunction(struct hs_State* H, hs_CFunction function, int argCount) {
  push(H, NEW_OBJ(newCFunction(H, function, argCount)));
}

bool hs_isString(struct hs_State* H, int index) {
  Value* v = getValueAt(H, index);
  if (v == NULL) {
    return false;
  }
  return IS_STRING(*v);
}

const char* hs_toString(struct hs_State* H, int index, size_t* length) {
  Value* v = getValueAt(H, index);
  if (v == NULL) {
    if (length != NULL) {
      *length = 0;
    }
    return NULL;
  }

  struct GcString* str = toString(H, v);
  if (length != NULL) {
    *length = str->length;
  }
  return str->chars;
}
