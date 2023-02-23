#ifndef _LOCAL_VARIABLE_INITIALISATIONS_H_
#define _LOCAL_VARIABLE_INITIALISATIONS_H_

#include "types.h"

extern node *LVIfunbody(node *arg_node, info *arg_info);
extern node *LVIvardecl(node *arg_node, info *arg_info);

extern node *LVIinitializeLocalVariables(node *syntaxtree);

#endif