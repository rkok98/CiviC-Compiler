#ifndef _NORMALIZE_FOR_LOOPS_H_
#define _NORMALIZE_FOR_LOOPS_H_

#include "types.h"

extern node *NFLdoNormalizeForLoops(node *syntaxtree);

extern node *NFLfunbody (node * arg_node, info * arg_info);
extern node *NFLfor (node * arg_node, info * arg_info);
extern node *NFLstmts (node * arg_node, info * arg_info);

extern node *NFLvarlet(node * arg_node, info * arg_info);
extern node *NFLvar(node * arg_node, info * arg_info);

#endif