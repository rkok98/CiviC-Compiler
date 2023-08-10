#ifndef _HELPERS_H_
#define _HELPERS_H_

#include "types.h"

extern char *HprintType(type type);
extern char *HprintBinOp(binop op);
extern char *HprintMonOp(monop op);

extern bool HisBooleanOperator(binop operator);

#endif