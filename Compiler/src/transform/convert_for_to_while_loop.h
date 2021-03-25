#ifndef _CONVERT_FOR_TO_WHILE_H_
#define _CONVERT_FOR_TO_WHILE_H_

#include "types.h"

extern node *CFTWfor(node *arg_node, info *arg_info);
extern node *CFTWstatements(node *arg_node, info *arg_info);

extern node *CFTWconvertForToWhile( node *syntaxtree);

#endif