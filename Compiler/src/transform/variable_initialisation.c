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
};

#define INFO_SYMBOL_TABLE(n) ((n)->symbol_table)
#define INFO_INIT_FUNCTION(n) ((n)->init_function)

static info *MakeInfo(void)
{
    info *result;

    DBUG_ENTER("MakeInfo");

    result = (info *)MEMmalloc(sizeof(info));

    INFO_SYMBOL_TABLE(result) = NULL;
    INFO_INIT_FUNCTION(result) = NULL;

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

    node *init_body = TBmakeFunbody(NULL, NULL, NULL);
    node *init_function = TBmakeFundef(T_void, STRcpy("__init"), init_body, NULL);

    node *init_symbol_table = TBmakeSymboltable(1, PROGRAM_SYMBOLTABLE(arg_node), NULL);
    FUNDEF_SYMBOLTABLE(init_function) = init_symbol_table;

    INFO_INIT_FUNCTION(arg_info) = init_function;

    node *declarations = TRAVdo(PROGRAM_DECLS(arg_node), arg_info);

    PROGRAM_DECLS(arg_node) = TBmakeDecls(init_function, declarations);

    DBUG_RETURN(arg_node);
}

node *VIglobdef(node *arg_node, info *arg_info)
{
    DBUG_ENTER("VIglobdef");

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