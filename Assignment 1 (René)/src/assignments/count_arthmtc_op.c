/*****************************************************************************
 *
 * Module: count_arthmtc_op
 *
 * Prefix: CAO
 *
 * Description:
 *
 * This module implements a traversal of the abstract syntax tree that
 * counts all arithmetic operators and inserts these into a module node.
 *
 * Author: René Kok (13671146)
 *         Doorstroomminor Software Engineering UvA
 *
 *****************************************************************************/


#include "count_arthmtc_op.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "memory.h"
#include "ctinfo.h"

struct INFO {
    int add;
    int sub;
    int mul;
    int div;
    int mod;
};

#define INFO_ADD(n)  ((n)->add)
#define INFO_SUB(n)  ((n)->sub)
#define INFO_MUL(n)  ((n)->mul)
#define INFO_DIV(n)  ((n)->div)
#define INFO_MOD(n)  ((n)->mod)

static info *MakeInfo(void) {
    info *result;

    DBUG_ENTER("MakeInfo");

    result = (info *) MEMmalloc(sizeof(info));

    INFO_ADD(result) = 0;
    INFO_SUB(result) = 0;
    INFO_MUL(result) = 0;
    INFO_DIV(result) = 0;
    INFO_MOD(result) = 0;

    DBUG_RETURN(result);
}

static info *FreeInfo(info *info) {
    DBUG_ENTER("FreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

/*
 * Analyzes a binary operator node and adds 1 to the amount of the right operator
 */
node *CAObinop(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAObinop");

    if (BINOP_OP( arg_node) == BO_add) {
        INFO_ADD( arg_info) += 1;
    }

    if (BINOP_OP( arg_node) == BO_sub) {
        INFO_SUB( arg_info) += 1;
    }

    if (BINOP_OP( arg_node) == BO_mul) {
        INFO_MUL( arg_info) += 1;
    }

    if (BINOP_OP( arg_node) == BO_div) {
        INFO_DIV( arg_info) += 1;
    }

    if (BINOP_OP( arg_node) == BO_mod) {
        INFO_MOD( arg_info) += 1;
    }

    DBUG_RETURN(arg_node);
}

node *CAOmodule(node *arg_node, info *arg_info) {
    DBUG_ENTER("CAOmodule");

    info *info = MakeInfo();

    TRAVdo(MODULE_NEXT(arg_node), info);

    MODULE_ADD(arg_node) = INFO_ADD(info);
    MODULE_SUB(arg_node) = INFO_SUB(info);
    MODULE_MUL(arg_node) = INFO_MUL(info);
    MODULE_DIV(arg_node) = INFO_DIV(info);
    MODULE_MOD(arg_node) = INFO_MOD(info);

    DBUG_RETURN(arg_node);
}

/*
 * Traversal start function
 */
node *CAOdoCountBinOp(node *syntaxtree) {
    info *arg_info;

    DBUG_ENTER("CAOdoCountBinOp");

    arg_info = MakeInfo();

    TRAVpush(TR_cao);
    syntaxtree = TRAVdo(syntaxtree, arg_info);
    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
