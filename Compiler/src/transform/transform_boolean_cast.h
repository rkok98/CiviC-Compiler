#ifndef _BOOLEAN_CAST_H_
#define _BOOLEAN_CAST_H_

#include "types.h"

extern node *TBCbinop(node *arg_node, info *arg_info);
extern node *TBCcast(node *arg_node, info *arg_info);

extern node *TBCnum(node *arg_node, info *arg_info);
extern node *TBCfloat(node *arg_node, info *arg_info);
extern node *TBCbool(node *arg_node, info *arg_info);
extern node *TBCvar(node *arg_node, info *arg_info);

extern node *TBCtransformBooleanCast(node *syntaxtree);

#endif