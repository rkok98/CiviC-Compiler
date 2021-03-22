#ifndef _COUNT_ARTHMTC_OP_H_
#define _COUNT_ARTHMTC_OP_H_

#include "types.h"

extern node *CAObinop(node *arg_node, info *arg_info);

extern node *CAOmodule(node *arg_node, info *arg_info);

extern node *CAOdoCountBinOp(node *syntaxtree);

#endif
