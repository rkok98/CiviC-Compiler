#include "string.h"
#include "symbol_table.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "memory.h"
#include "free.h"
#include "str.h"
#include "ctinfo.h"

node *STinsert(node *symbol_table, node *entry)
{
    DBUG_ENTER("STinsert");
    if (find(symbol_table, SYMBOLTABLEENTRY_NAME(entry) != NULL))
    {
        CTIerror("Redefinition of var %s at line %d, column %d", SYMBOLTABLEENTRY_NAME(entry), NODE_LINE(entry), NODE_COL(entry));
        return NULL;
    }

    node *last = STlast(entry);

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

node *STfind(node *symbol_table, const char *entry)
{
    DBUG_ENTER("STfind");

    if (symbol_table == NULL)
    {
        return NULL;
    }

    if (SYMBOLTABLEENTRY_TABLE(symbol_table) != NULL)
    {
        return STsearchVariableEntry(SYMBOLTABLEENTRY_NEXT(symbol_table), entry);
    }

    if (strcmp(SYMBOLTABLEENTRY_NAME(symbol_table), entry) != 0)
    {
        return STsearchVariableEntry(SYMBOLTABLEENTRY_NEXT(symbol_table), entry);
    }

    DBUG_RETURN(symbol_table);
}

node *STlast(node *symbol_table)
{
    if (!SYMBOLTABLEENTRY_NEXT(symbol_table))
    {
        return NULL;
    }

    node *entry = SYMBOLTABLEENTRY_NEXT(symbol_table);

    while (SYMBOLTABLEENTRY_NEXT(entry))
    {
        entry = SYMBOLTABLEENTRY_NEXT(entry);
    }

    return entry;
}

void STprint(node *symbol_table)
{
    STdisplayEntry(SYMBOLTABLE_ENTRY(symbol_table), 0);
}

void STprintentry(node *symbol_table, size_t tabs)
{
    if (symbol_table == NULL)
        return;

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

    if (SYMBOLTABLEENTRY_TABLE(symbol_table) != NULL)
        STdisplay(SYMBOLTABLEENTRY_TABLE(symbol_table), tabs + 1);

    STdisplayEntry(SYMBOLTABLEENTRY_NEXT(symbol_table), tabs);
}