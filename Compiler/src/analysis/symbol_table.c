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

    if (NODE_TYPE(SYMBOLTABLEENTRY_DECLARATION(entry)) == node_type)
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

    SYMBOLTABLEENTRY_OFFSET(entry) = STcountByType(SYMBOLTABLE_ENTRIES(symbol_table), NODE_TYPE(SYMBOLTABLEENTRY_DECLARATION(entry)));

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

node *STfindByDecl(node *symbol_table, node *decl)
{
    DBUG_ENTER("STfindByDecl");
    node *entry = SYMBOLTABLE_ENTRIES(symbol_table);

    while (entry)
    {
        node *entry_decl = SYMBOLTABLEENTRY_DECLARATION(entry);

        if (NODE_TYPE(entry_decl) == NODE_TYPE(decl))
        {
            if (NODE_TYPE(entry_decl) == N_globdef && STReq(GLOBDEF_NAME(entry_decl), GLOBDEF_NAME(decl)))
            {
                DBUG_RETURN(entry);
            }
            if (NODE_TYPE(entry_decl) == N_globdecl && STReq(GLOBDECL_NAME(entry_decl), GLOBDECL_NAME(decl)))
            {
                DBUG_RETURN(entry);
            }
            if (NODE_TYPE(entry_decl) == N_fundef && STReq(FUNDEF_NAME(entry_decl), FUNDEF_NAME(decl)))
            {
                DBUG_RETURN(entry);
            }
            if (NODE_TYPE(entry_decl) == N_fundecl && STReq(FUNDECL_NAME(entry_decl), FUNDECL_NAME(decl)))
            {
                DBUG_RETURN(entry);
            }
            if (NODE_TYPE(entry_decl) == N_vardecl && STReq(VARDECL_NAME(entry_decl), VARDECL_NAME(decl)))
            {
                DBUG_RETURN(entry);
            }
            if (NODE_TYPE(entry_decl) == N_param && STReq(PARAM_NAME(entry_decl), PARAM_NAME(decl)))
            {
                DBUG_RETURN(entry);
            }
        }

        entry = SYMBOLTABLEENTRY_NEXT(entry);
    }

    DBUG_RETURN(NULL);
}

node *STfindByDeclInParents(node *symbol_table, node *decl)
{
    DBUG_ENTER("STfindByDeclInParents");

    node *entry = STfindByDecl(symbol_table, decl);

    if (entry)
    {
        DBUG_RETURN(entry);
    }

    if (SYMBOLTABLE_PARENT(symbol_table))
    {
        DBUG_RETURN(STfindByDeclInParents(SYMBOLTABLE_PARENT(symbol_table), decl));
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
