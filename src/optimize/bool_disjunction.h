#ifndef _BOOL_DISJUNCTION_AND_CONJUNCTION_H_
#define _BOOL_DISJUNCTION_AND_CONJUNCTION_H_

#include "types.h"

extern node *BDCbinop (node * arg_node, info * arg_info);

extern node *BDCdoBoolDisjunction(node *syntaxtree);

#endif