#ifndef _VARIABLE_INITIALISATIONS_H_
#define _VARIABLE_INITIALISATIONS_H_

#include "types.h"

extern node *VIprogram(node *arg_node, info *arg_info);
extern node *VIglobdef(node *arg_node, info *arg_info);

extern node *CVinitializeVariables(node *syntaxtree);

#endif