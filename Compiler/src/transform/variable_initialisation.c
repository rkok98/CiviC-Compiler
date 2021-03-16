#include "variable_initialisation.h"

#include "symbol_table.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "memory.h"
#include "free.h"
#include "str.h"
#include "ctinfo.h"
#include "copy.h"
#include "types.h"

struct INFO
{
    node *symbol_table;
    node *init_function;
    node *first_statement;
    node *last_statement;
};

#define INFO_SYMBOL_TABLE(n) ((n)->symbol_table)
#define INFO_INIT_FUNCTION(n) ((n)->init_function)
#define INFO_FIRST_STATEMENT(n) ((n)->first_statement)
#define INFO_LAST_STATEMENT(n) ((n)->last_statement)

static info *MakeInfo(void)
{
    info *result;

    DBUG_ENTER("MakeInfo");

    result = (info *)MEMmalloc(sizeof(info));

    INFO_SYMBOL_TABLE(result) = NULL;
    INFO_INIT_FUNCTION(result) = NULL;
    INFO_FIRST_STATEMENT(result) = NULL;
    INFO_LAST_STATEMENT(result) = NULL;

    DBUG_RETURN(result);
}

static info *FreeInfo(info *info)
{
    DBUG_ENTER("FreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node *VIprogram(node *arg_node, info *arg_info)
{
    DBUG_ENTER("VIprogram");

    // set the symbol table
    INFO_SYMBOL_TABLE(arg_info) = PROGRAM_SYMBOLTABLE(arg_node);

    // traverse over the decls
    PROGRAM_DECLS(arg_node) = TRAVopt(PROGRAM_DECLS(arg_node), arg_info);

    // get the added statements
    node *stmts = INFO_FIRST_STATEMENT(arg_info);

    // do we need to append statements?
    if (stmts == NULL) {
        DBUG_RETURN(arg_node);
    }

    // Create the __init function
    node *funbod = TBmakeFunbody(NULL, NULL, stmts);
    node *init = TBmakeFundef(T_void, STRcpy("__init"), funbod, NULL);
    FUNDEF_ISEXPORT(init) = 1;

    // prepend the __init function to other DECLS
    PROGRAM_DECLS(arg_node) = TBmakeDecls(init, PROGRAM_DECLS(arg_node));

    // refernce to the symbol table
    node *table = INFO_SYMBOL_TABLE(arg_info);

    // create a new symbol table for this function definition
    node *inittable = TBmakeSymboltable(1, NULL, NULL);
    SYMBOLTABLE_PARENT(inittable) = table;

    // create the symbol table
    node *entry = TBmakeSymboltableentry(STRcpy(FUNDEF_NAME(init)), FUNDEF_TYPE(init), 1, 0, 0, NULL, inittable);

    // add the entry to the symbol table
    STinsert(table, entry);

    DBUG_RETURN(arg_node);
}

node *VIglobdef(node *arg_node, info *arg_info)
{
    DBUG_ENTER("VIglobdef");

    node *expr = GLOBDEF_INIT(arg_node);

    // do we have expressions?
    if (expr == NULL) DBUG_RETURN(arg_node);
    
    node *varlet = TBmakeVarlet(STRcpy(GLOBDEF_NAME(arg_node)), arg_node, NULL);
    node *assign = TBmakeAssign(varlet, COPYdoCopy(expr));

    FREEdoFreeTree(expr);
    GLOBDEF_INIT(arg_node) = NULL;

    if (INFO_FIRST_STATEMENT(arg_info) == NULL)
    {
        INFO_FIRST_STATEMENT(arg_info) = TBmakeStmts(assign, NULL);
        INFO_LAST_STATEMENT(arg_info) = INFO_FIRST_STATEMENT(arg_info);
    }
    else
    {
        node *node = TBmakeStmts(assign, NULL);
        STMTS_NEXT(INFO_LAST_STATEMENT(arg_info)) = node;
        INFO_LAST_STATEMENT(arg_info) = node;
    }

    DBUG_RETURN(arg_node);
}

node *VIinitializeVariables(node *syntaxtree)
{
    DBUG_ENTER("CVinitializeVariables");

    info *info = MakeInfo();

    TRAVpush(TR_vi);
    syntaxtree = TRAVdo(syntaxtree, info);
    TRAVpop();

    FreeInfo(info);

    DBUG_RETURN(syntaxtree);
}