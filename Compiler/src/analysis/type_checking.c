#include "type_checking.h"

#include "helpers.h"
#include "symbol_table.h"

#include "ctinfo.h"
#include "dbug.h"
#include "free.h"
#include "memory.h"
#include "str.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"

struct INFO
{
    node *symbol_table;
    type type;
    type return_type;
};

#define INFO_SYMBOL_TABLE(n) ((n)->symbol_table)
#define INFO_TYPE(n) ((n)->type)
#define INFO_RETURN_TYPE(n) ((n)->return_type)

static info *MakeInfo(void)
{
    info *result;
    DBUG_ENTER("MakeInfo");

    result = (info *)MEMmalloc(sizeof(info));

    INFO_SYMBOL_TABLE(result) = NULL;
    INFO_TYPE(result) = T_unknown;
    INFO_RETURN_TYPE(result) = T_unknown;

    DBUG_RETURN(result);
}

static info *FreeInfo(info *info)
{
    DBUG_ENTER("FreeInfo");
    info = MEMfree(info);
    DBUG_RETURN(info);
}

unsigned int TCcountArguments(node *arg_node)
{
    if (!arg_node)
    {
        return 0;
    }

    return 1 + TCcountArguments(EXPRS_NEXT(arg_node));
}

node *TCnum(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCnum");
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

    node *symbol_table = INFO_SYMBOL_TABLE(arg_info);
    type return_type = INFO_RETURN_TYPE(arg_info);

    INFO_SYMBOL_TABLE(arg_info) = FUNDEF_SYMBOLTABLE(arg_node);
    INFO_RETURN_TYPE(arg_info) = FUNDEF_TYPE(arg_node);

    FUNDEF_FUNBODY(arg_node) = TRAVdo(FUNDEF_FUNBODY(arg_node), arg_info);

    INFO_SYMBOL_TABLE(arg_info) = symbol_table;
    INFO_RETURN_TYPE(arg_info) = return_type;

    DBUG_RETURN(arg_node);
}

node *TCvardecl(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCvardecl");

    if (VARDECL_INIT(arg_node))
    {
        VARDECL_INIT(arg_node) = TRAVdo(VARDECL_INIT(arg_node), arg_info);

        type vardecl_expected_type = VARDECL_TYPE(arg_node);
        type vardecl_actual_type = INFO_TYPE(arg_info);

        if (vardecl_actual_type != vardecl_expected_type)
        {
            CTIerrorLine(NODE_LINE(arg_node) + 1, "Expected type '%s' but actual '%s'", HprintType(vardecl_expected_type), HprintType(vardecl_actual_type));
        }
    }

    VARDECL_NEXT(arg_node) = TRAVopt(VARDECL_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *TCvarlet(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCvarlet");

    node *entry = STfindInParents(INFO_SYMBOL_TABLE(arg_info), VARLET_NAME(arg_node));
    INFO_TYPE(arg_info) = SYMBOLTABLEENTRY_TYPE(entry);

    DBUG_RETURN(arg_node);
}

node *TCassign(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCassign");

    ASSIGN_LET(arg_node) = TRAVdo(ASSIGN_LET(arg_node), arg_info);
    type assign_expected_type = INFO_TYPE(arg_info);

    ASSIGN_EXPR(arg_node) = TRAVdo(ASSIGN_EXPR(arg_node), arg_info);
    type assign_actual_type = INFO_TYPE(arg_info);

    if (assign_actual_type != assign_expected_type)
    {
        CTIerrorLine(NODE_LINE(arg_node) + 1, "Expected type '%s' but actual type '%s'", HprintType(assign_expected_type), HprintType(assign_actual_type));
    }

    DBUG_RETURN(arg_node);
}

node *TCreturn(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCreturn");

    type return_expected_type = INFO_RETURN_TYPE(arg_info);
    type return_actual_type = T_void;

    if (RETURN_EXPR(arg_node))
    {
        RETURN_EXPR(arg_node) = TRAVdo(RETURN_EXPR(arg_node), arg_info);
        return_actual_type = INFO_TYPE(arg_info);
    }

    if (return_actual_type != return_expected_type)
    {
        CTIerrorLine(NODE_LINE(arg_node) + 1, "Expected type '%s' but actual '%s'", HprintType(return_expected_type), HprintType(return_actual_type));
    }

    DBUG_RETURN(arg_node);
}

node *TCfuncall(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCfuncall");

    node *fundecl_entry = STfindFuncInParents(INFO_SYMBOL_TABLE(arg_info), FUNCALL_NAME(arg_node));

    if (!fundecl_entry)
    {
        CTIerrorLine(NODE_LINE(arg_node) + 1, "Function '%s' called but is not declared", FUNCALL_NAME(arg_node));
        DBUG_RETURN(arg_node);
    }

    FUNCALL_ARGS(arg_node) = TRAVopt(FUNCALL_ARGS(arg_node), arg_info);
    
    INFO_TYPE(arg_info) = SYMBOLTABLEENTRY_TYPE(fundecl_entry);

    DBUG_RETURN(arg_node);
}

node *TCcast(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCcast");

    CAST_EXPR(arg_node) = TRAVdo(CAST_EXPR(arg_node), arg_info);

    if (INFO_TYPE(arg_info) == T_void)
    {
        CTIerrorLine(NODE_LINE(arg_node), "Cannot cast '%s' to '%s'", HprintType(INFO_TYPE(arg_info)), HprintType(CAST_TYPE(arg_node)));
    }

    INFO_TYPE(arg_info) = CAST_TYPE(arg_node);

    DBUG_RETURN(arg_node);
}

node *TCbinop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCbinop");

    binop binop_op = BINOP_OP(arg_node);

    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);
    type binop_left_type = INFO_TYPE(arg_info);

    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);
    type binop_right_type = INFO_TYPE(arg_info);

    // Validate if the left and right operand types are the same type.
    // There's an exception for boolean and integer, a binary operator is compatible
    // with a mix of both types.
    if (binop_left_type != binop_right_type && 
       (binop_left_type != T_bool && binop_left_type != T_int) && 
       (binop_right_type != T_bool && binop_right_type != T_int))
    {
        CTIerrorLine(NODE_LINE(arg_node) + 1, "Cannot apply operator '%s' to type '%s' and type '%s'", HprintBinOp(binop_op), HprintType(binop_left_type), HprintType(binop_right_type));
    }

    if (binop_op == BO_mod && binop_right_type != T_int)
    {

        CTIerrorLine(NODE_LINE(arg_node) + 1, "Cannot apply operator '%s' to type '%s' and type '%s'", HprintBinOp(binop_op), HprintType(binop_left_type), HprintType(binop_right_type));
    }

    if (HisBooleanOperator(BINOP_OP(arg_node)))
    {
        INFO_TYPE(arg_info) = T_bool;
    }

    DBUG_RETURN(arg_node);
}

node *TCdoTypeChecking(node *syntaxtree)
{
    DBUG_ENTER("TCdoTypecheck");

    info *arg_info = MakeInfo();

    TRAVpush(TR_tc);
    syntaxtree = TRAVdo(syntaxtree, arg_info);
    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}