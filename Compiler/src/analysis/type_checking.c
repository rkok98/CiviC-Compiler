#include "type_checking.h"
#include "symbol_table.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"

#include "memory.h"
#include "free.h"
#include "str.h"
#include "ctinfo.h"

struct INFO
{
    node *symbol_table;
    type return_type;
    type type;
};

#define INFO_SYMBOL_TABLE(n) ((n)->symbol_table)
#define INFO_RETURN_TYPE(n) ((n)->return_type)
#define INFO_TYPE(n) ((n)->type)

static info *MakeInfo()
{
    info *result;

    DBUG_ENTER("MakeInfo");

    result = (info *)MEMmalloc(sizeof(info));
    INFO_SYMBOL_TABLE(result) = NULL;
    INFO_RETURN_TYPE(result) = T_unknown;
    INFO_TYPE(result) = T_unknown;

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

node *TCreturn(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCreturn");

    type expected_type = INFO_RETURN_TYPE(arg_info);

    if (RETURN_EXPR(arg_node) == NULL && expected_type == T_void)
    {
        DBUG_RETURN(arg_node);
    }

    RETURN_EXPR(arg_node) = TRAVopt(RETURN_EXPR(arg_node), arg_info);

    if (INFO_RETURN_TYPE(arg_info) == INFO_TYPE(arg_info))
    {
        // TODO ERROR
        CTIerrorLine(1, "KANKER");
    }

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

node *TCnum(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCint");
    INFO_TYPE(arg_info) = T_int;
    DBUG_RETURN(arg_node);
}

node *TCfloat(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCfloat");
    INFO_TYPE(arg_info) = T_float;
    DBUG_RETURN(arg_node);
}

node *TCbool(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCbool");
    INFO_TYPE(arg_info) = T_bool;
    DBUG_RETURN(arg_node);
}