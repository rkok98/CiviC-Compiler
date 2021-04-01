#ifndef _SYMBOL_TABLE_
#define _SYMBOL_TABLE_

#include "types.h"

extern node *STinsert(node *symbol_table, node *entry);

extern unsigned int STcountParams(node *entry);
extern unsigned int STcountVarDecls(node *entry);

extern node *STfind(node *symbol_table, char *name);
extern node *STfindInParents(node *symbol_table, char *name);

extern node *STlast(node *symbol_table);

// TODO
extern node *STsearchFundef(node *table, const char *name);
extern node *STdeepSearchFundef(node *table, const char *name);

extern node *STdeepSearchVariableByName(node *table, const char *name);

extern node *STdeepSearchByNode(node *table, node *link);

#endif