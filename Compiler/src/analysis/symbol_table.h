#ifndef _SYMBOL_TABLE_
#define _SYMBOL_TABLE_

#include "types.h"

extern node *STinsert(node *symbol_table, node *entry);

extern node *STfind(node *symbol_table, char *name);
extern node *STfindInParents(node *symbol_table, char *name);

extern node *STlast(node *symbol_table);

extern void STprint(node *symbol_table);
void STprintentry(node *symbol_table, size_t tabs);
void STprintindentation(size_t n);
const char *STentrytype(type type);


// TODO
extern node *STsearchFundef(node *table, const char *name);
extern node *STdeepSearchFundef(node *table, const char *name);

extern size_t STparams();
extern size_t STVardecls(node *table);

extern node *STdeepSearchVariableByName(node *table, const char *name);

extern node *STdeepSearchByNode(node *table, node *link);

#endif