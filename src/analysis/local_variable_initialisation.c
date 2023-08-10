#include "local_variable_initialisation.h"

#include "copy.h"
#include "ctinfo.h"
#include "dbug.h"
#include "free.h"
#include "memory.h"
#include "str.h"
#include "symbol_table.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"

/**
 * Information structure to track the first and last statements
 * in a given code block or function body
 */
struct INFO
{
    node *first_statement;
    node *last_statement;
};

#define INFO_FIRST_STATEMENT(n) ((n)->first_statement)
#define INFO_LAST_STATEMENT(n) ((n)->last_statement)

static info *MakeInfo(void)
{
    info *result;

    DBUG_ENTER("MakeInfo");

    result = (info *)MEMmalloc(sizeof(info));

    INFO_LAST_STATEMENT(result) = NULL;
    INFO_FIRST_STATEMENT(result) = NULL;

    DBUG_RETURN(result);
}

static info *FreeInfo(info *info)
{
    DBUG_ENTER("FreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

/**
 * Traverse function for function bodies.
 * It processes the variable declarations inside the function body
 */
node *LVIfunbody(node *arg_node, info *arg_info)
{
    DBUG_ENTER("LVIfunbody");

    info *funbody_info = MakeInfo();

    // Traverse variable declarations inside the function body
    TRAVopt(FUNBODY_VARDECLS(arg_node), funbody_info);

    // If there's a last statement in the funbody_info, update the statement pointers
    if (INFO_LAST_STATEMENT(funbody_info))
    {
        STMTS_NEXT(INFO_LAST_STATEMENT(funbody_info)) = FUNBODY_STMTS(arg_node);
        FUNBODY_STMTS(arg_node) = INFO_FIRST_STATEMENT(funbody_info);
    }

    funbody_info = FreeInfo(funbody_info);

    DBUG_RETURN(arg_node);
}

/**
 * Traverse function for variable declarations.
 * It ensures that local variables are properly initialized
 */
node *LVIvardecl(node *arg_node, info *arg_info)
{
    DBUG_ENTER("LVIvardecl");

    node *vardecl_init = VARDECL_INIT(arg_node);

    // If no variable initialization is found, continue traversing to next declaration
    if (!vardecl_init)
    {
        VARDECL_NEXT(arg_node) = TRAVopt(VARDECL_NEXT(arg_node), arg_info);
        DBUG_RETURN(arg_node);
    }

    // Create a new assignment node to assign the initialization value to the variable
    node *vardecl_varlet = TBmakeVarlet(STRcpy(VARDECL_NAME(arg_node)), arg_node, NULL);
    node *vardecl_assign = TBmakeAssign(vardecl_varlet, COPYdoCopy(vardecl_init));

    // Free the memory of the initial value as it's no longer needed
    // and set the initialization of variable declaration to NULL
    FREEdoFreeTree(vardecl_init);
    VARDECL_INIT(arg_node) = NULL;

    // Create a new statement node for the assignment
    node *node = TBmakeStmts(vardecl_assign, NULL);

    // Update the INFO structure fields to keep track of the statements
    if (INFO_FIRST_STATEMENT(arg_info) == NULL)
    {
        INFO_FIRST_STATEMENT(arg_info) = node;
        INFO_LAST_STATEMENT(arg_info) = node;
    }
    else
    {
        STMTS_NEXT(INFO_LAST_STATEMENT(arg_info)) = node;
        INFO_LAST_STATEMENT(arg_info) = node;
    }

    // Continue traversal for the next variable declaration
    VARDECL_NEXT(arg_node) = TRAVopt(VARDECL_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *LVIinitializeLocalVariables(node *syntaxtree)
{
    DBUG_ENTER("LVIinitializeLocalVariables");

    TRAVpush(TR_lvi);
    syntaxtree = TRAVdo(syntaxtree, NULL);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}