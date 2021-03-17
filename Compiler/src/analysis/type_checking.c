#include "type_checking.h"
#include "symbol_table.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "free.h"
#include "str.h"

#include "memory.h"
#include "ctinfo.h"

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
            CTIerrorLine(NODE_LINE(arg_node), "Expected type: %s but actual: %s at %d:%d", get_type(vardecl_expected_type), get_type(vardecl_actual_type));
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
        CTIerrorLine(NODE_LINE(arg_node), "Expected type: %s but actual type: %s", get_type(assign_expected_type), get_type(assign_actual_type));
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
        CTIerrorLine(NODE_LINE(arg_node), "Expected type: %s but actual: %s at %d:%d", get_type(return_expected_type), get_type(return_actual_type));
    }

    DBUG_RETURN(arg_node);
}

/**
node *TCfuncall(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCfuncall");

    char *name = FUNCALL_NAME(arg_node);
    node *entry = STfindInParents(INFO_SYMBOL_TABLE(arg_info), name);

    node *current_param = SYMBOLTABLEENTRY_PARAMS(entry);
    char *expected = STRcat(name, "(");
    while (current_param)
    {
        expected = STRcat(expected, get_type(PARAM_TYPE(current_param)));
        if (PARAM_NEXT(current_param))
        {
            expected = STRcat(expected, ", ");
        }
        current_param = PARAM_NEXT(current_param);
    }
    expected = STRcat(expected, ")");

    node *current_exprs = FUNCALL_PARAMS(arg_node);
    char *actual = STRcat(name, "(");
    while (current_exprs)
    {
        EXPRS_EXPR(current_exprs) = TRAVdo(EXPRS_EXPR(current_exprs), arg_info);
        actual = STRcat(actual, get_type(INFO_TYPE(arg_info)));
        if (EXPRS_NEXT(current_exprs))
        {
            actual = STRcat(actual, ", ");
        }
        current_exprs = EXPRS_NEXT(current_exprs);
    }
    actual = STRcat(actual, ")");

    if (!STReq(actual, expected))
    {
        funcalltype_error(expected, actual, NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    INFO_TYPE(arg_info) = SYMBOLTABLEENTRY_TYPE(entry);

    DBUG_RETURN(arg_node);
}
*/

node *TCcast(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCcast");

    CAST_EXPR(arg_node) = TRAVdo(CAST_EXPR(arg_node), arg_info);

    if (INFO_TYPE(arg_info) == T_void)
    {
        CTIerrorLine(NODE_LINE(arg_node), "Cannot cast %s to %s.", get_type(INFO_TYPE(arg_info)), get_type(CAST_TYPE(arg_node)));
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

    if (binop_left_type != binop_right_type)
    {
        CTIerrorLine(NODE_LINE(arg_node), "Cannot apply %s to type %s and type %s at %d:%d", get_binop(binop_op), get_type(binop_left_type), get_type(binop_right_type));
    }

    if (binop_op == BO_mod && binop_right_type != T_int)
    {
        CTIerrorLine(NODE_LINE(arg_node), "Cannot apply %s to type %s and type %s at %d:%d", get_binop(binop_op), get_type(binop_left_type), get_type(binop_right_type));
    }

    if (isBooleanOperator(BINOP_OP(arg_node)))
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

bool isBooleanOperator(binop operator)
{
    return
    operator== BO_lt ||
    operator== BO_le ||
    operator== BO_gt ||
    operator== BO_ge ||
    operator== BO_eq ||
    operator== BO_ne ||
    operator== BO_or ||
    operator== BO_and;
}

char *get_type(type type)
{
    switch (type)
    {
    case T_void:
        return "void";
    case T_bool:
        return "bool";
    case T_int:
        return "int";
    case T_float:
        return "float";
    case T_unknown:
        return "unknown";
    default:
        return NULL;
    }
}

char *get_binop(binop op)
{
    switch (op)
    {
    case BO_add:
        return "+";
    case BO_sub:
        return "-";
    case BO_mul:
        return "*";
    case BO_div:
        return "/";
    case BO_mod:
        return "%%";
    case BO_lt:
        return "<";
    case BO_le:
        return "<=";
    case BO_gt:
        return ">";
    case BO_ge:
        return ">=";
    case BO_eq:
        return "==";
    case BO_ne:
        return "!=";
    case BO_or:
        return "||";
    case BO_and:
        return "&&";
    case BO_unknown:
        return "unknown";
    }
    return NULL;
}

char *get_monop(monop op)
{
    switch (op)
    {
    case MO_not:
        return "!";
    case MO_neg:
        return "-";
    case MO_unknown:
        return "unknown";
    default:
        return NULL;
    }
}