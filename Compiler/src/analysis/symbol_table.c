#include "string.h"
#include "symbol_table.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "memory.h"
#include "ctinfo.h"
#include "str.h"

unsigned int STcountByType(node *entry, nodetype node_type)
{
    if (entry == NULL)
    {
        return 0;
    }

    if (NODE_TYPE(SYMBOLTABLEENTRY_DEFINITION(entry)) == node_type)
    {
        return 1 + STcountByType(SYMBOLTABLEENTRY_NEXT(entry), node_type);
    }

    return 0 + STcountByType(SYMBOLTABLEENTRY_NEXT(entry), node_type);
}

unsigned int STcountParams(node *entry)
{
    if (!entry)
    {
        return 0;
    }

    if (SYMBOLTABLEENTRY_ISPARAMETER(entry))
    {
        return 1 + STcountParams(SYMBOLTABLEENTRY_NEXT(entry));
    }

    return 0 + STcountParams(SYMBOLTABLEENTRY_NEXT(entry));
}

unsigned int STcountVarDecls(node *entry)
{
    if (!entry)
    {
        return 0;
    }

    if (!SYMBOLTABLEENTRY_ISPARAMETER(entry))
    {
        return 1 + STcountVarDecls(SYMBOLTABLEENTRY_NEXT(entry));
    }

    return 0 + STcountVarDecls(SYMBOLTABLEENTRY_NEXT(entry));
}

node *STinsert(node *symbol_table, node *entry)
{
    DBUG_ENTER("STinsert");

    if (STfind(symbol_table, SYMBOLTABLEENTRY_NAME(entry)) != NULL)
    {
        CTIerrorLine(NODE_LINE(entry) + 1, "Variable/Function '%s' is already defined.", SYMBOLTABLEENTRY_NAME(entry));
        return NULL;
    }

    SYMBOLTABLEENTRY_OFFSET(entry) = STcountByType(SYMBOLTABLE_ENTRIES(symbol_table), NODE_TYPE(SYMBOLTABLEENTRY_DEFINITION(entry)));

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

node *STfindInParents(node *symbol_table, char *name)
{
    DBUG_ENTER("STfindInParents");

    node *entry = STfind(symbol_table, name);

    if (entry)
    {
        DBUG_RETURN(entry);
    }

    if (SYMBOLTABLE_PARENT(symbol_table))
    {
        DBUG_RETURN(STfindInParents(SYMBOLTABLE_PARENT(symbol_table), name));
    }

    DBUG_RETURN(NULL);
}

node *STfindFunc(node *symbol_table, char *name)
{
    DBUG_ENTER("STfindFunc");
    node *entry = SYMBOLTABLE_ENTRIES(symbol_table);

    while (entry)
    {
        if (SYMBOLTABLEENTRY_ISFUNCTION(entry) && STReq(SYMBOLTABLEENTRY_NAME(entry), name))
        {
            DBUG_RETURN(entry);
        }

        entry = SYMBOLTABLEENTRY_NEXT(entry);
    }

    DBUG_RETURN(NULL);
}

node *STfindFuncInParents(node *symbol_table, char *name)
{
    DBUG_ENTER("STfindFuncInParents");

    node *entry = STfindFunc(symbol_table, name);

    if (entry)
    {
        DBUG_RETURN(entry);
    }

    if (SYMBOLTABLE_PARENT(symbol_table))
    {
        DBUG_RETURN(STfindFuncInParents(SYMBOLTABLE_PARENT(symbol_table), name));
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

// TODO
node *STdeepSearchByNode(node *table, node *link)
{
    // get the entry
    node *entry = SYMBOLTABLE_ENTRIES(table);

    // loop over the entries
    for (; entry != NULL; entry = SYMBOLTABLEENTRY_NEXT(entry))
    {
        node *n = SYMBOLTABLEENTRY_DEFINITION(entry);

        DBUG_PRINT("GBC", ("SEARCH 1.1"));

        if (NODE_TYPE(link) != NODE_TYPE(n))
            continue;

        if (NODE_TYPE(n) == N_globdef && STReq(GLOBDEF_NAME(n), GLOBDEF_NAME(link)))
            return entry;
        if (NODE_TYPE(n) == N_globdecl && STReq(GLOBDECL_NAME(n), GLOBDECL_NAME(link)))
            return entry;
        if (NODE_TYPE(n) == N_fundef && STReq(FUNDEF_NAME(n), FUNDEF_NAME(link)))
            return entry;
        if (NODE_TYPE(n) == N_fundecl && STReq(FUNDECL_NAME(n), FUNDECL_NAME(link)))
            return entry;
        if (NODE_TYPE(n) == N_vardecl && STReq(VARDECL_NAME(n), VARDECL_NAME(link)))
            return entry;
        if (NODE_TYPE(n) == N_param && STReq(PARAM_NAME(n), PARAM_NAME(link)))
            return entry;

        DBUG_PRINT("GBC", ("SEARCH 2.1"));
    }

    if (SYMBOLTABLE_PARENT(table) == NULL)
        return NULL;

    return STdeepSearchByNode(SYMBOLTABLE_PARENT(table), link);
}
