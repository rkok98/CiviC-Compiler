#ifndef _TYPE_CHECKING_H_
#define _TYPE_CHECKING_H_

#include "types.h"

extern node *TCprogram(node *arg_node, info *arg_info);
extern node *TCfundef(node *arg_node, info *arg_info);

extern node *TCreturn(node *arg_node, info *arg_info);

extern node *TCnum(node *arg_node, info *arg_info);
extern node *TCfloat(node *arg_node, info *arg_info);
extern node *TCbool(node *arg_node, info *arg_info);

extern node *TCdoTypeChecking(node *syntaxtree);

#endif