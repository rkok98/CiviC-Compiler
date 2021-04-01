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
/**
 *  Find an entry in a linked list of Symbol Table Entry nodes
 *  @param  list        the symbol table entry node
 *  @param  name        the name of the node
 *  @param  type        the type of node
 *  @return node|NULL
 */
node *STsearchFundefEntry(node *list, const char *name)
{
    // do we have a valid entry
    if (list == NULL)
        return NULL;

    // skip fundefs
    if (SYMBOLTABLEENTRY_TABLE(list) == NULL)
        return STsearchFundefEntry(SYMBOLTABLEENTRY_NEXT(list), name);

    // check if the name is the same
    if (strcmp(SYMBOLTABLEENTRY_NAME(list), name) != 0)
        return STsearchFundefEntry(SYMBOLTABLEENTRY_NEXT(list), name);

    // return the result
    return list;
}

/**
 *  Find an entry in a linked list of Symbol Table Entry nodes
 *  @param  table       the symbol table
 *  @param  name        the name of the node
 *  @param  type        the type of node
 *  @return node|NULL
 */
node *STsearchFundef(node *table, const char *name)
{
    // the entry
    node *entry = SYMBOLTABLE_ENTRIES(table);

    // return the result
    return STsearchFundefEntry(entry, name);
}

/**
 *  Find an entry  by its name in a linked list of Symbol Table Entry nodes
 *  @param  table       the symbol table
 *  @param  name        the name of the node
 *  @return node|NULL
 */
node *STdeepSearchFundef(node *table, const char *name)
{
    // search for the node in the current scope
    node *found = STsearchFundef(table, name);

    // do we have a node?
    if (found != NULL)
        return found;

    // get the parent table
    node *parent = SYMBOLTABLE_PARENT(table);

    // do we have a parent table?
    if (parent == NULL)
        return NULL;

    // search for the node in the parent table
    return STdeepSearchFundef(parent, name);
}

node *STsearchVariableEntry(node *list, const char *name, type type)
{
    // do we have a valid entry
    if (list == NULL)
        return NULL;

    // skip fundefs
    if (SYMBOLTABLEENTRY_TABLE(list) != NULL)
        return STsearchVariableEntry(SYMBOLTABLEENTRY_NEXT(list), name, type);

    // check if the name is the same
    if (strcmp(SYMBOLTABLEENTRY_NAME(list), name) != 0)
        return STsearchVariableEntry(SYMBOLTABLEENTRY_NEXT(list), name, type);

    // check if the type is the same
    if (type != T_unknown && SYMBOLTABLEENTRY_TYPE(list) != type)
        return STsearchVariableEntry(SYMBOLTABLEENTRY_NEXT(list), name, type);

    // return the result
    return list;
}

node *STsearchVariable(node *table, const char *name, type type)
{
    // the entry
    node *entry = SYMBOLTABLE_ENTRIES(table);

    // return the result
    return STsearchVariableEntry(entry, name, type);
}

node *STsearchVariableByName(node *table, const char *name)
{
    // the entry
    node *entry = SYMBOLTABLE_ENTRIES(table);

    // return the result
    return STsearchVariableEntry(entry, name, T_unknown);
}

node *STdeepSearchVariableByName(node *table, const char *name)
{
    // search for the node in the current scope
    node *found = STsearchVariableByName(table, name);

    // do we have a node?
    if (found != NULL)
        return found;

    // get the parent table
    node *parent = SYMBOLTABLE_PARENT(table);

    // do we have a parent table?
    if (parent == NULL)
        return NULL;

    // search for the node in the parent table
    return STdeepSearchVariableByName(parent, name);
}

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
