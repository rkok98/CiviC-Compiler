#ifndef _SYMBOL_TABLE_
#define _SYMBOL_TABLE_

#include "types.h"

extern node *STinsert(node *symbol_table, node *entry);

extern node *STfind(node *symbol_table, char *name, int *store);

extern node *STlast(node *symbol_table);

extern void STprint(node *symbol_table);
void STprintentry(node *symbol_table, size_t tabs);
void STprintindentation(size_t n);
const char *STentrytype(type type);

#endif