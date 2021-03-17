#include "type_checking.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "symbol_table.h"

#include "free.h"
#include "str.h"

#include "memory.h"
#include "ctinfo.h"

struct INFO
{
    type type;
    type return_type;
    node *symboltable;
};

#define INFO_TYPE(n) ((n)->type)
#define INFO_RETURNTYPE(n) ((n)->return_type)
#define CURRENT_SYMBOLTABLE(n) ((n)->symboltable)

static info *MakeInfo(void)
{
    info *result;
    DBUG_ENTER("MakeInfo");

    result = (info *)MEMmalloc(sizeof(info));

    INFO_TYPE(result) = T_unknown;
    INFO_RETURNTYPE(result) = T_unknown;
    CURRENT_SYMBOLTABLE(result) = NULL;

    DBUG_RETURN(result);
}

static info *FreeInfo(info *info)
{
    DBUG_ENTER("FreeInfo");
    info = MEMfree(info);
    DBUG_RETURN(info);
}

/* ************************************************* */

void type_error(type expected, type actual, int line, int col)
{
    CTIerror("TypeError: Expected %s, but found %s at line %d, column %d", get_type(expected), get_type(actual), line, col);
}

void binop_unequal_type_error(type left, type right, binop op, int line, int col)
{
    CTIerror("TypeError: Tried to apply %s to unequal types %s and %s at line %d, column %d", get_binop(op), get_type(left), get_type(right), line, col);
}

void binop_unsupported_type_error(type left, type right, binop op, int line, int col)
{
    CTIerror("TypeError: Tried to apply %s to unsupported types %s and %s at line %d, column %d", get_binop(op), get_type(left), get_type(right), line, col);
}

void monoptype_error(type type, monop op, int line, int col)
{
    CTIerror("TypeError: Tried to apply %s to type %s at line %d, column %d", get_monop(op), get_type(type), line, col);
}

void funcalltype_error(char *expected, char *actual, int line, int col)
{
    CTIerror("TypeError: Expected %s, but found %s at line %d, column %d", expected, actual, line, col);
}

void casttype_error(type cast_type, type expr_type, int line, int col)
{
    CTIerror("TypeError: Attempting to cast %s to %s at line %d, column %d", get_type(expr_type), get_type(cast_type), line, col);
}

void assign_for_induction_var_error(int line, int col)
{
    CTIerror("Error: Attempting to assign for induction var at line %d, column %d", line, col);
}

/* ************************************************* */

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
/**
node *TCfuncall(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCfuncall");

    char *name = FUNCALL_NAME(arg_node);
    node *entry = STfindInParents(CURRENT_SYMBOLTABLE(arg_info), name);

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
        casttype_error(CAST_TYPE(arg_node), INFO_TYPE(arg_info), NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    INFO_TYPE(arg_info) = CAST_TYPE(arg_node);

    DBUG_RETURN(arg_node);
}

node *TCmonop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCmonop");

    monop op = MONOP_OP(arg_node);
    MONOP_OPERAND(arg_node) = TRAVdo(MONOP_OPERAND(arg_node), arg_info);
    type type = INFO_TYPE(arg_info);

    if ((op == MO_not && type != T_bool) || (op == MO_neg && type == T_bool))
    {
        monoptype_error(type, op, NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    DBUG_RETURN(arg_node);
}

node *TCbinop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCbinop");

    binop op = BINOP_OP(arg_node);

    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);
    type left_type = INFO_TYPE(arg_info);

    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);
    type right_type = INFO_TYPE(arg_info);

    if (left_type != right_type)
    {
        binop_unequal_type_error(left_type, right_type, op, NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    if ((op == BO_sub || op == BO_div || op == BO_lt || op == BO_le || op == BO_gt || op == BO_ge) && right_type == T_bool)
    {
        binop_unsupported_type_error(left_type, right_type, op, NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    if (op == BO_mod && right_type != T_int)
    {
        binop_unsupported_type_error(left_type, right_type, op, NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    if (isBooleanOperator(BINOP_OP(arg_node)))
    {
        INFO_TYPE(arg_info) = T_bool;
    }

    DBUG_RETURN(arg_node);
}

node *TCassign(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCassign");

    ASSIGN_LET(arg_node) = TRAVdo(ASSIGN_LET(arg_node), arg_info);
    type expected_type = INFO_TYPE(arg_info);

    ASSIGN_EXPR(arg_node) = TRAVdo(ASSIGN_EXPR(arg_node), arg_info);
    type actual_type = INFO_TYPE(arg_info);

    if (actual_type != expected_type)
    {
        printf("%s: Expected: %s, ACTUAL: %s\n", VARLET_NAME(ASSIGN_LET(arg_node)), get_type(expected_type), get_type(actual_type));
        type_error(expected_type, actual_type, NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    if (STReq(STRsubStr(VARLET_NAME(ASSIGN_LET(arg_node)), 0, 4), "_for"))
    {
        assign_for_induction_var_error(NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    DBUG_RETURN(arg_node);
}

node *TCvarlet(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCvarlet");

    node *entry = STfindInParents(CURRENT_SYMBOLTABLE(arg_info), VARLET_NAME(arg_node));

    INFO_TYPE(arg_info) = SYMBOLTABLEENTRY_TYPE(entry);

    DBUG_RETURN(arg_node);
}

node *TCvardecl(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCvardec");

    if (VARDECL_INIT(arg_node))
    {
        VARDECL_INIT(arg_node) = TRAVdo(VARDECL_INIT(arg_node), arg_info);

        type expected_type = VARDECL_TYPE(arg_node);
        type actual_type = INFO_TYPE(arg_info);

        if (actual_type != expected_type)
        {
            type_error(expected_type, actual_type, NODE_LINE(arg_node), NODE_COL(arg_node));
        }
    }

    VARDECL_NEXT(arg_node) = TRAVopt(VARDECL_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *TCifelse(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCifelse");

    IFELSE_COND(arg_node) = TRAVdo(IFELSE_COND(arg_node), arg_info);

    type expected_type = T_bool;
    type actual_type = INFO_TYPE(arg_info);
    if (actual_type != expected_type)
    {
        type_error(expected_type, actual_type, NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    /* Traverse child nodes */
    IFELSE_THEN(arg_node) = TRAVdo(IFELSE_THEN(arg_node), arg_info);
    IFELSE_ELSE(arg_node) = TRAVopt(IFELSE_ELSE(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *TCwhile(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCwhile");

    WHILE_COND(arg_node) = TRAVdo(WHILE_COND(arg_node), arg_info);

    type expected_type = T_bool;
    type actual_type = INFO_TYPE(arg_info);
    if (actual_type != expected_type)
    {
        type_error(expected_type, actual_type, NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    /* Traverse child nodes */
    WHILE_BLOCK(arg_node) = TRAVdo(WHILE_BLOCK(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *TCdowhile(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCdowhile");

    DOWHILE_COND(arg_node) = TRAVdo(DOWHILE_COND(arg_node), arg_info);

    type expected_type = T_bool;
    type actual_type = INFO_TYPE(arg_info);
    if (actual_type != expected_type)
    {
        type_error(expected_type, actual_type, NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    /* Traverse child nodes */
    DOWHILE_BLOCK(arg_node) = TRAVdo(DOWHILE_BLOCK(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *TCreturn(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCreturn");

    type expected_type = INFO_RETURNTYPE(arg_info);
    type actual_type = T_void;

    if (RETURN_EXPR(arg_node))
    {
        RETURN_EXPR(arg_node) = TRAVdo(RETURN_EXPR(arg_node), arg_info);
        actual_type = INFO_TYPE(arg_info);
    }

    if (actual_type != expected_type)
    {
        type_error(expected_type, actual_type, NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    DBUG_RETURN(arg_node);
}

/* ************************************************* */

node *TCfor(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCfor");

    type expected_type = T_int;

    FOR_START(arg_node) = TRAVdo(FOR_START(arg_node), arg_info);
    type actual_type = INFO_TYPE(arg_info);
    if (actual_type != expected_type)
    {
        type_error(expected_type, actual_type, NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    FOR_STOP(arg_node) = TRAVdo(FOR_STOP(arg_node), arg_info);
    actual_type = INFO_TYPE(arg_info);
    if (actual_type != expected_type)
    {
        type_error(expected_type, actual_type, NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    FOR_STEP(arg_node) = TRAVopt(FOR_STEP(arg_node), arg_info);
    actual_type = INFO_TYPE(arg_info);
    if (actual_type != expected_type)
    {
        type_error(expected_type, actual_type, NODE_LINE(arg_node), NODE_COL(arg_node));
    }

    /* Traverse other child nodes */
    FOR_BLOCK(arg_node) = TRAVdo(FOR_BLOCK(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *TCfundef(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCfundef");

    node *previous = CURRENT_SYMBOLTABLE(arg_info);
    CURRENT_SYMBOLTABLE(arg_info) = FUNDEF_SYMBOLTABLE(arg_node);

    type previous_rettype = INFO_RETURNTYPE(arg_info);

    INFO_RETURNTYPE(arg_info) = FUNDEF_TYPE(arg_node);

    /* Traverse child nodes */
    FUNDEF_FUNBODY(arg_node) = TRAVdo(FUNDEF_FUNBODY(arg_node), arg_info);

    CURRENT_SYMBOLTABLE(arg_info) = previous;

    INFO_RETURNTYPE(arg_info) = previous_rettype;
    
    DBUG_RETURN(arg_node);
}

node *TCprogram(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCprogram");

    node *previous = CURRENT_SYMBOLTABLE(arg_info);
    CURRENT_SYMBOLTABLE(arg_info) = PROGRAM_SYMBOLTABLE(arg_node);

    /* Traverse the declarations */
    PROGRAM_DECLS(arg_node) = TRAVdo(PROGRAM_DECLS(arg_node), arg_info);

    CURRENT_SYMBOLTABLE(arg_info) = previous;

    DBUG_RETURN(arg_node);
}

/* ************************************************* */

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