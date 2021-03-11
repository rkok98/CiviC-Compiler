#include "string.h"
#include "symbol_table.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "memory.h"
#include "ctinfo.h"
#include "str.h"

node *STinsert(node *symbol_table, node *entry)
{
    DBUG_ENTER("STinsert");

    if (STfind(symbol_table, SYMBOLTABLEENTRY_NAME(entry), NULL) != NULL)
    {
        CTIerror("Redefinition of var %s at line %d, column %d", SYMBOLTABLEENTRY_NAME(entry), NODE_LINE(entry), NODE_COL(entry));
        return NULL;
    }

    node *last = STlast(symbol_table);

    if (!last)
    {
        SYMBOLTABLE_ENTRIES(symbol_table) = entry;
    }
    else
    {
        SYMBOLTABLEENTRY_NEXT(last) = entry;
    }

    DBUG_RETURN(entry);
}

node *STfind(node *symbol_table, char *name, int *store)
{
    DBUG_ENTER("STfind");
    node *entry = SYMBOLTABLE_ENTRIES(symbol_table);

    if (store)
    {
        *store = 0;
    }

    while (entry)
    {
        if (STReq(SYMBOLTABLEENTRY_NAME(entry), name))
        {
            DBUG_RETURN(entry);
        }

        entry = SYMBOLTABLEENTRY_NEXT(entry);

        if (store)
        {
            (*store)++;
        }
    }

    if (store)
    {
        *store = -1;
    }

    DBUG_RETURN(NULL);
}

node *STlast(node *symbol_table)
{
    DBUG_ENTER("STlast");
    
    if (!SYMBOLTABLE_ENTRIES(symbol_table)) {
        DBUG_RETURN(NULL);
    }

    node *entry = SYMBOLTABLE_ENTRIES(symbol_table);
    
    while (SYMBOLTABLEENTRY_NEXT(entry)) {
        entry = SYMBOLTABLEENTRY_NEXT(entry);
    }

    DBUG_RETURN(entry);
}

void STprint(node *symbol_table)
{
    STprintentry(SYMBOLTABLE_ENTRIES(symbol_table), 0);
}

void STprintentry(node *symbol_table, size_t tabs)
{
    if (symbol_table == NULL) {
        return;
    }

    for (size_t i = 0; i < tabs; i++)
    {
        printf("\t");
    }

    printf("Type: ");
    switch (SYMBOLTABLEENTRY_TYPE(symbol_table))
    {
    case T_void:
        printf("void");
        break;
    case T_bool:
        printf("bool");
        break;
    case T_int:
        printf("int");
        break;
    case T_float:
        printf("float");
        break;
    case T_unknown:
        DBUG_ASSERT(0, "unknown type detected!");
    }

    printf(", Name: %s\n", SYMBOLTABLEENTRY_NAME(symbol_table));

    STprintentry(SYMBOLTABLEENTRY_NEXT(symbol_table), tabs);
}