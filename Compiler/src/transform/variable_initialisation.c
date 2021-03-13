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
    node *init_function = TBmakeFundef(T_void, "__init", init_body, NULL);

    // CONTINUE

    DBUG_ENTER(arg_node);
}

node *VIglobdef(node *arg_node, info *arg_info)
{
    return arg_node;
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