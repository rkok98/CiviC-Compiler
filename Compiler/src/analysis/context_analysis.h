#ifndef _CONTEXT_ANALYSIS_
#define _CONTEXT_ANALYSIS_

#include "types.h"

node *CAprogram(node *arg_node, info *arg_info);
node *CAglobdecl(node *arg_node, info *arg_info);
node *CAglobdef(node *arg_node, info *arg_info);

node *CAfundecl(node *arg_node, info *arg_info);
node *CAfundef(node *arg_node, info *arg_info);
node *CAparam(node *arg_node, info *arg_info);

node *CAvardecl(node *arg_node, info *arg_info);
node *CAvarlet(node *arg_node, info *arg_info);
node *CAvar(node *arg_node, info *arg_info);

extern node *CAdoContextAnalysis(node *syntaxtree);

#endif