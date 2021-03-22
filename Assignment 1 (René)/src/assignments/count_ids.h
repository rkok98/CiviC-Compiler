#ifndef _COUNT_IDS_H
#define _COUNT_IDS_H

#include "types.h"

extern node *CIDvar(node *arg_node, info *arg_info);

extern node *CIDvarlet(node *arg_node, info *arg_info);

extern node *CIDdoCountIds(node *syntaxtree);

#endif
