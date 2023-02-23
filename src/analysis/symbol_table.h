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

extern node *STfindByDecl(node *symbol_table, node *decl);
extern node *STfindByDeclInParents(node *symbol_table, node *decl);

extern node *STlast(node *symbol_table);

#endif