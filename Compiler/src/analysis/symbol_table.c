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

    if (STfind(symbol_table, SYMBOLTABLEENTRY_NAME(entry)) != NULL)
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

node *STfind(node *symbol_table, char *name)
{
    DBUG_ENTER("STfind");
    node *entry = SYMBOLTABLE_ENTRIES(symbol_table);

    while (entry)
    {
        if (STReq(SYMBOLTABLEENTRY_NAME(entry), name))
        {
            DBUG_RETURN(entry);
        }

        entry = SYMBOLTABLEENTRY_NEXT(entry);
    }

    DBUG_RETURN(NULL);
}

node *STlast(node *symbol_table)
{
    DBUG_ENTER("STlast");

    if (!SYMBOLTABLE_ENTRIES(symbol_table))
    {
        DBUG_RETURN(NULL);
    }

    node *entry = SYMBOLTABLE_ENTRIES(symbol_table);

    while (SYMBOLTABLEENTRY_NEXT(entry))
    {
        entry = SYMBOLTABLEENTRY_NEXT(entry);
    }

    DBUG_RETURN(entry);
}

void STprint(node *symbol_table)
{
    printf("\n");
    STprintindentation(SYMBOLTABLE_NESTINGLEVEL(symbol_table));
    printf("%-10s %-10s %-10s\n", "Symbol:", "Type:", "Scope Level:");
    STprintentry(SYMBOLTABLE_ENTRIES(symbol_table), SYMBOLTABLE_NESTINGLEVEL(symbol_table));
}

void STprintentry(node *entry, size_t nesting_level)
{
    if (entry == NULL)
    {
        return;
    }

    STprintindentation(nesting_level);
    printf("%-10s %-10s %-10zu\n", SYMBOLTABLEENTRY_NAME(entry), STentrytype(SYMBOLTABLEENTRY_TYPE(entry)), nesting_level);

    if (SYMBOLTABLEENTRY_NEXTTABLE(entry))
    {
        STprint(SYMBOLTABLEENTRY_NEXTTABLE(entry));
    }

    STprintentry(SYMBOLTABLEENTRY_NEXT(entry), nesting_level);
}

void STprintindentation(size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        printf("\t");
    }
}

const char *STentrytype(type type)
{
    switch (type)
    {
    case T_void:
        return "void";
    case T_bool:
        return "bool";
    case T_int:
        return "int";
    case T_float:
        return "float";
    case T_unknown:
        DBUG_ASSERT(0, "unknown type detected!");
        return "Unknown";
    }
}