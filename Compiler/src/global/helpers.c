#include "types.h"
#include "helpers.h"

char *HprintType(type type)
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

char *HprintBinOp(binop op)
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

char *HprintMonOp(monop op)
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

bool HisBooleanOperator(binop operator)
{
    return
    operator== BO_lt  ||
    operator== BO_le  ||
    operator== BO_gt  ||
    operator== BO_ge  ||
    operator== BO_eq  ||
    operator== BO_ne  ||
    operator== BO_and ||
    operator== BO_or;
}
