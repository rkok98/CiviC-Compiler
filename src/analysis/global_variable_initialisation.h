#ifndef _GLOBAL_VARIABLE_INITIALISATIONS_H_
#define _GLOBAL_VARIABLE_INITIALISATIONS_H_

#include "types.h"

extern node *GVIprogram(node *arg_node, info *arg_info);
extern node *GVIglobdef(node *arg_node, info *arg_info);

extern node *GVIinitializeGlobalVariables(node *syntaxtree);

#endif