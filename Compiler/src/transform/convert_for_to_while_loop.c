/* Rewrite all for loops to while loops */

#include "convert_for_to_while_loop.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "free.h"
#include "copy.h"
#include "str.h"

#include "helpers.h"

/* Append statements to a block */
void append_to_block(node *block, node *new_statements)
{
    node *current = block;

    while (STMTS_NEXT(current))
    {
        current = STMTS_NEXT(current);
    }

    if (!current)
    {
        STMTS_STMT(block) = new_statements;
    }
    else
    {
        STMTS_NEXT(current) = new_statements;
    }
}

node *CFTWfor(node *arg_node, info *arg_info)
{
    DBUG_ENTER("FTWfor");

    FOR_START(arg_node) = TRAVopt(FOR_START(arg_node), arg_info);
    FOR_STEP(arg_node) = TRAVopt(FOR_STEP(arg_node), arg_info);
    FOR_STOP(arg_node) = TRAVopt(FOR_STOP(arg_node), arg_info);

    /* Create while expression */
    binop binop = BO_lt;
    node *step = FOR_STEP(arg_node);
    if (step && NODE_TYPE(step) == N_num && NUM_VALUE(step) < 0)
    {
        binop = BO_gt;
    }

    node *while_expr = TBmakeBinop(binop, TBmakeVar(STRcpy(FOR_LOOPVAR(arg_node)), NULL, NULL),
                                   TBmakeVar(STRcat(FOR_LOOPVAR(arg_node), "_end"), NULL, NULL));

    /* Create increment statement */
    node *right_expr = TBmakeNum(1);
    if (FOR_STEP(arg_node))
    {
        right_expr = COPYdoCopy(FOR_STEP(arg_node));
    }

    node *inc = TBmakeBinop(BO_add, TBmakeVar(STRcpy(FOR_LOOPVAR(arg_node)), NULL, NULL), right_expr);
    node *a_inc = TBmakeAssign(inc, TBmakeVar(STRcpy(FOR_LOOPVAR(arg_node)), NULL, NULL));

    FOR_BLOCK(arg_node) = TRAVdo(FOR_BLOCK(arg_node), arg_info);

    /* Add last statements to block */
    append_to_block(FOR_BLOCK(arg_node), TBmakeStmts(a_inc, NULL));
    node *newBlock = COPYdoCopy(FOR_BLOCK(arg_node));

    /* Create while node, and replace for */
    arg_node = FREEdoFreeTree(arg_node);
    arg_node = TBmakeWhile(while_expr, newBlock);

    DBUG_RETURN(arg_node);
}

node *CFTWstatements(node *arg_node, info *arg_info)
{
    DBUG_ENTER("FTWstatements");

    node *statement = STMTS_STMT(arg_node);
    if (NODE_TYPE(statement) == N_for)
    {
        node *start_assign = TBmakeAssign(COPYdoCopy(FOR_START(statement)),
                                          TBmakeVar(STRcpy(FOR_LOOPVAR(statement)), NULL, NULL));
        node *end_assign = TBmakeAssign(COPYdoCopy(FOR_STOP(statement)),
                                        TBmakeVar(STRcat(FOR_LOOPVAR(statement), "_end"), NULL, NULL));
        node *end_statements = TBmakeStmts(end_assign, arg_node);
        node *start_statements = TBmakeStmts(start_assign, end_statements);

        STMTS_STMT(arg_node) = TRAVdo(STMTS_STMT(arg_node), arg_info);
        STMTS_NEXT(arg_node) = TRAVopt(STMTS_NEXT(arg_node), arg_info);
        DBUG_RETURN(start_statements);
    }

    STMTS_STMT(arg_node) = TRAVdo(STMTS_STMT(arg_node), arg_info);
    STMTS_NEXT(arg_node) = TRAVopt(STMTS_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CFTWconvertForToWhile(node *syntaxtree)
{
    DBUG_ENTER("CFTWconvertForToWhile");

    TRAVpush(TR_cftw);
    syntaxtree = TRAVdo(syntaxtree, NULL);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}