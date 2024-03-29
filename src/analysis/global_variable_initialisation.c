#include "global_variable_initialisation.h"

#include "ctinfo.h"
#include "copy.h"
#include "dbug.h"
#include "free.h"
#include "memory.h"
#include "str.h"
#include "symbol_table.h"
#include "traverse.h"
#include "tree_basic.h"
#include "types.h"

struct INFO
{
    node *init_function;
    node *last_statement;
};

#define INFO_INIT_FUNCTION(n) ((n)->init_function)
#define INFO_LAST_STATEMENT(n) ((n)->last_statement)

static info *MakeInfo(void)
{
    info *result;

    DBUG_ENTER("MakeInfo");

    result = (info *)MEMmalloc(sizeof(info));

    INFO_INIT_FUNCTION(result) = NULL;
    INFO_LAST_STATEMENT(result) = NULL;

    DBUG_RETURN(result);
}

static info *FreeInfo(info *info)
{
    DBUG_ENTER("FreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node *GVIprogram(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GVIprogram");

    node *init_body = TBmakeFunbody(NULL, NULL, NULL);
    node *init_function = TBmakeFundef(T_void, STRcpy("__init"), init_body, NULL);
    node *init_symbol_table = TBmakeSymboltable(1, PROGRAM_SYMBOLTABLE(arg_node), NULL);
    
    INFO_INIT_FUNCTION(arg_info) = init_function;
    node *entry = TBmakeSymboltableentry(STRcpy(FUNDEF_NAME(init_function)), FUNDEF_TYPE(init_function), arg_node, init_symbol_table, NULL);

    SYMBOLTABLEENTRY_ISFUNCTION(entry) = TRUE;
    SYMBOLTABLEENTRY_ISEXPORT(entry) = FUNDEF_ISEXPORT(init_function);
    SYMBOLTABLEENTRY_ISPARAMETER(entry) = FALSE;

    FUNDEF_SYMBOLTABLE(init_function) = init_symbol_table;
    FUNDEF_ISEXPORT(init_function) = TRUE;

    node *declarations = TRAVdo(PROGRAM_DECLS(arg_node), arg_info);

    if (INFO_LAST_STATEMENT(arg_info))
    {
        PROGRAM_DECLS(arg_node) = TBmakeDecls(init_function, declarations);
    }
    else
    {
        FREEdoFreeTree(init_function);
    }

    STinsert(PROGRAM_SYMBOLTABLE(arg_node), entry);

    DBUG_RETURN(arg_node);
}

node *GVIglobdef(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GVIglobdef");

    node *globdef_init = GLOBDEF_INIT(arg_node);
    globdef_init = TRAVopt(globdef_init, arg_info);

    if (globdef_init)
    {
        node *init_function = INFO_INIT_FUNCTION(arg_info);

        node *globdef_varlet = TBmakeVarlet(STRcpy(GLOBDEF_NAME(arg_node)), arg_node, NULL);
        node *globdef_assign = TBmakeAssign(globdef_varlet, COPYdoCopy(globdef_init));

        node *new_statement = TBmakeStmts(globdef_assign, NULL);

        node *last_statement = INFO_LAST_STATEMENT(arg_info);
        if (!last_statement)
        {
            FUNBODY_STMTS(FUNDEF_FUNBODY(init_function)) = new_statement;
        }
        else
        {
            STMTS_NEXT(last_statement) = new_statement;
        }

        INFO_LAST_STATEMENT(arg_info) = new_statement;
        GLOBDEF_INIT(arg_node) = NULL;
    }

    DBUG_RETURN(arg_node);
}

node *GVIinitializeGlobalVariables(node *syntaxtree)
{
    DBUG_ENTER("GVIinitializeGlobalVariables");

    info *info = MakeInfo();

    TRAVpush(TR_gvi);
    syntaxtree = TRAVdo(syntaxtree, info);
    TRAVpop();

    FreeInfo(info);

    DBUG_RETURN(syntaxtree);
}
