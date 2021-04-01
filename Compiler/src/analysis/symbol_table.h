#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include "types.h"

extern unsigned int STcountParams(node *entry);
extern unsigned int STcountVarDecls(node *entry);

extern node *STinsert(node *symbol_table, node *entry);

extern node *STfind(node *symbol_table, char *name);
extern node *STfindInParents(node *symbol_table, char *name);

extern node *STfindFunc(node *symbol_table, char *name);
extern node *STfindFuncInParents(node *symbol_table, char *name);

extern node *STlast(node *symbol_table);

// TODO
extern node *STdeepSearchByNode(node *table, node *link);

#endif