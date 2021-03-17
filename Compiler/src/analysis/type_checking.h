#ifndef _TYPE_CHECKING_H_
#define _TYPE_CHECKING_H_

#include "types.h"

node *TCnum(node *arg_node, info *arg_info);
node *TCfloat(node *arg_node, info *arg_info);
node *TCbool(node *arg_node, info *arg_info);
node *TCfuncall(node *arg_node, info *arg_info);
node *TCcast(node *arg_node, info *arg_info);
node *TCmonop(node *arg_node, info *arg_info);
node *TCbinop(node *arg_node, info *arg_info);

/* ************************************************* */

node *TCassign(node *arg_node, info *arg_info);
node *TCvardecl(node *arg_node, info *arg_info);
node *TCvarlet(node *arg_node, info *arg_info);
node *TCifelse(node *arg_node, info *arg_info);
node *TCwhile(node *arg_node, info *arg_info);
node *TCdowhile(node *arg_node, info *arg_info);
node *TCfor(node *arg_node, info *arg_info);
node *TCreturn(node *arg_node, info *arg_info);

/* ************************************************* */

node *TCfor(node *arg_node, info *arg_info);
node *TCfundef(node *arg_node, info *arg_info);
node *TCprogram(node *arg_node, info *arg_info);

extern node *TCdoTypeChecking(node *syntaxtree);

bool isBooleanOperator(binop operator);
char *get_type(type type);
char *get_binop(binop op);
char *get_monop(monop op);

#endif