#include "type_checking.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "str.h"
#include "memory.h"

struct INFO
{
    node *symbol_table;
    type *return_type;
};

#define INFO_SYMBOL_TABLE(n) ((n)->symbol_table)
#define INFO_RETURN_TYPE(n) ((n)->return_type)

static info *MakeInfo()
{
    info *result;

    DBUG_ENTER("MakeInfo");

    result = (info *)MEMmalloc(sizeof(info));
    INFO_SYMBOL_TABLE(result) = NULL;
    INFO_RETURN_TYPE(result) = NULL;

    DBUG_RETURN(result);
}

static info *FreeInfo(info *info)
{
    DBUG_ENTER("FreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node *TCprogram(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCprogram");

    INFO_SYMBOL_TABLE(arg_info) = PROGRAM_SYMBOLTABLE(arg_node);
    PROGRAM_DECLS(arg_node) = TRAVdo(PROGRAM_DECLS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *TCfundef(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCfundef");

    info *fundef_info = MakeInfo();
    node *fundef_table = FUNDEF_SYMBOLTABLE(arg_node);

    INFO_SYMBOL_TABLE(fundef_info) = fundef_table;

    FUNDEF_FUNBODY(arg_node) = TRAVopt(FUNDEF_FUNBODY(arg_node), fundef_info);

    DBUG_RETURN(arg_node);
}

node *TCdoTypeChecking(node *syntaxtree)
{
    info *arg_info;

    DBUG_ENTER("TCdoTypeChecking");

    arg_info = MakeInfo();

    TRAVpush(TR_tc);
    syntaxtree = TRAVdo(syntaxtree, NULL);
    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}