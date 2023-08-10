#include "symbol_table.h"

#include "ctinfo.h"
#include "dbug.h"
#include "memory.h"
#include "str.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"

/**
 * Counts the number of symbol table entries that match the given node type.
 *
 * @param entry Start of the symbol table entry list.
 * @param node_type The node type to be matched against.
 * @return The count of symbol table entries of the specified type.
 */
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

    return STcountByType(SYMBOLTABLEENTRY_NEXT(entry), node_type);
}

/**
 * Counts the number of parameters in the symbol table entries.
 *
 * @param entry Start of the symbol table entry list.
 * @return The count of parameters in the symbol table.
 */
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

    return STcountParams(SYMBOLTABLEENTRY_NEXT(entry));
}

/**
 * Counts the number of variable declarations in the symbol table entries.
 *
 * @param entry Start of the symbol table entry list.
 * @return The count of variable declarations in the symbol table.
 */
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

    return STcountVarDecls(SYMBOLTABLEENTRY_NEXT(entry));
}

/**
 * Inserts a new symbol table entry into the given symbol table.
 * If the entry name already exists in the table, it prints an error.
 *
 * @param symbol_table The symbol table to which the entry should be added.
 * @param entry The symbol table entry to be inserted.
 * @return The inserted entry if successful, otherwise NULL.
 */
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

/**
 * Searches for a symbol table entry with the given name.
 *
 * @param symbol_table The symbol table to search within.
 * @param name The name of the entry to search for.
 * @return The found entry if exists, otherwise NULL.
 */
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

/**
 * Searches for a symbol table entry with the given name, including parent tables.
 *
 * @param symbol_table The symbol table to start the search from.
 * @param name The name of the entry to search for.
 * @return The found entry if exists, otherwise NULL.
 */
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

/**
 * Searches for a function entry with the given name in the symbol table.
 *
 * @param symbol_table The symbol table to search within.
 * @param name The name of the function entry to search for.
 * @return The found function entry if exists, otherwise NULL.
 */
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

/**
 * Searches for a function entry with the given name in the symbol table, including parent tables.
 *
 * @param symbol_table The symbol table to start the search from.
 * @param name The name of the function entry to search for.
 * @return The found function entry if exists, otherwise NULL.
 */
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

/**
 * Searches for a symbol table entry with the given declaration in the symbol table.
 *
 * @param symbol_table The symbol table to search within.
 * @param decl The declaration of the entry to search for.
 * @return The found entry if exists, otherwise NULL.
 */
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

/**
 * Searches for a symbol table entry with the given declaration in the symbol table, including parent tables.
 *
 * @param symbol_table The symbol table to start the search from.
 * @param decl The declaration of the entry to search for.
 * @return The found entry if exists, otherwise NULL.
 */
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

/**
 * Retrieves the last entry in the given symbol table.
 *
 * @param symbol_table The symbol table to retrieve the last entry from.
 * @return The last entry in the symbol table or NULL if the table is empty.
 */
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
