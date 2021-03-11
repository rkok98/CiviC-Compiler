#ifndef _SYMBOL_TABLE_
#define _SYMBOL_TABLE_

#include "types.h"

extern node *STinsert(node *symbol_table, node *entry);

extern node *STfind(node *symbol_table, const char *entry);

extern node *STlast(node *symbol_table);

extern void STprint(node *symbol_table);

#endif