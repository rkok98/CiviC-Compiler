#ifndef _FOR_LOOP_VARIABLE_INITIALISATION_H_
#define _FOR_LOOP_VARIABLE_INITIALISATION_H_

#include "types.h"

extern node *FLVIfunbody(node *arg_node, info *arg_info);
extern node *FLVIstmts(node *arg_node, info *arg_info);
extern node *FLVIfor(node *arg_node, info *arg_info);

extern node *FLVIvarlet(node *arg_node, info *arg_info);
extern node *FLVIvar(node *arg_node, info *arg_info);

extern node *FLVIinitializeForLoopsVariables(node *syntaxtree);

#endif