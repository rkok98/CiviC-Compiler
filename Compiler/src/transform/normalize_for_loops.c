#include "normalize_for_loops.h"
#include "key_value_linked_list.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "memory.h"
#include "free.h"
#include "str.h"
#include "copy.h"
#include "ctinfo.h"

#include "print.h"

struct INFO
{
    node *variable_declarations;
    node *statements;
    kvlistnode *induction_variables;

    unsigned int for_loop_counter;
};

#define INFO_VARDECLS(n) ((n)->variable_declarations)
#define INFO_STATEMENTS(n) ((n)->statements)
#define INFO_INDUCTION_VARIABLES(n) ((n)->induction_variables)
#define INFO_FOR_LOOP_COUNTER(n) ((n)->for_loop_counter)

void append(node *front, node *new)
{
    // no need to continue if the node is empty
    if (front == NULL)
        return;

    if (NODE_TYPE(front) == N_stmts)
    {
        // did we reach the end?
        if (STMTS_NEXT(front) == NULL)
            STMTS_NEXT(front) = new;

        // get the next node
        else
            append(STMTS_NEXT(front), new);
    }

    else if (NODE_TYPE(front) == N_vardecl)
    {
        // did we reach the end?
        if (VARDECL_NEXT(front) == NULL)
            VARDECL_NEXT(front) = new;

        // get the next node
        else
            append(VARDECL_NEXT(front), new);
    }
}

static info *MakeInfo()
{
    info *result;

    DBUG_ENTER("MakeInfo");

    result = (info *)MEMmalloc(sizeof(info));

    INFO_VARDECLS(result) = NULL;
    INFO_STATEMENTS(result) = NULL;
    INFO_INDUCTION_VARIABLES(result) = NULL;
    INFO_FOR_LOOP_COUNTER(result) = 0;

    DBUG_RETURN(result);
}

static info *FreeInfo(info *info)
{
    DBUG_ENTER("FreeInfo");

    KVLLdispose(INFO_INDUCTION_VARIABLES(info));

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node *NFLfunbody(node *arg_node, info *arg_info)
{
    DBUG_ENTER("NFLfunbody");

    info *funbody_info = MakeInfo();

    FUNBODY_STMTS(arg_node) = TRAVopt(FUNBODY_STMTS(arg_node), funbody_info);

    if (INFO_VARDECLS(funbody_info))
    {
        if (FUNBODY_VARDECLS(arg_node) == NULL)
        {
            FUNBODY_VARDECLS(arg_node) = INFO_VARDECLS(funbody_info);
        }
        else
        {
            append(FUNBODY_VARDECLS(arg_node), INFO_VARDECLS(funbody_info));
        }
    }

    funbody_info = FreeInfo(funbody_info);
    DBUG_RETURN(arg_node);
}

node *NFLstmts(node *arg_node, info *arg_info)
{
    DBUG_ENTER("NFLstmts");

    STMTS_STMT(arg_node) = TRAVdo(STMTS_STMT(arg_node), arg_info);

    if (NODE_TYPE(STMTS_STMT(arg_node)) == N_for)
    {
        node *oldnode = arg_node;
        append(INFO_STATEMENTS(arg_info), arg_node);
        arg_node = INFO_STATEMENTS(arg_info);
        INFO_STATEMENTS(arg_info) = NULL;

        STMTS_NEXT(oldnode) = TRAVopt(STMTS_NEXT(oldnode), arg_info);
    }
    else
    {
        STMTS_NEXT(arg_node) = TRAVopt(STMTS_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node *NFLfor(node *arg_node, info *arg_info)
{
    DBUG_ENTER("NFLfor");

    // set the new name
    char *name = STRcatn(4, "_for_", STRitoa(INFO_FOR_LOOP_COUNTER(arg_info)), "_", FOR_LOOPVAR(arg_node));
    INFO_FOR_LOOP_COUNTER(arg_info)
    ++;

    // add the name to the list
    if (!INFO_INDUCTION_VARIABLES(arg_info))
    {
        INFO_INDUCTION_VARIABLES(arg_info) = KVLLcreate(FOR_LOOPVAR(arg_node), name, NULL);
    }
    // prepend the the new head
    else
    {
        INFO_INDUCTION_VARIABLES(arg_info) = KVLLprepend(INFO_INDUCTION_VARIABLES(arg_info), FOR_LOOPVAR(arg_node), name);
    }

    // traverse over the nodes
    FOR_BLOCK(arg_node) = TRAVopt(FOR_BLOCK(arg_node), arg_info);

    // remove the node from the list
    INFO_INDUCTION_VARIABLES(arg_info) = KVLLremove_front(INFO_INDUCTION_VARIABLES(arg_info));

    // create a new vardecl node
    node *step = TBmakeVardecl(STRcat(name, "_step"), T_int, NULL, NULL, NULL);
    node *stop = TBmakeVardecl(STRcat(name, "_stop"), T_int, NULL, NULL, step);
    node *start = TBmakeVardecl(STRcpy(name), T_int, NULL, NULL, stop);

    // do we already have a front set?
    if (INFO_VARDECLS(arg_info) == NULL)
    {
        INFO_VARDECLS(arg_info) = start;
    }
    else
    {
        append(INFO_VARDECLS(arg_info), start);
    }

    // step expression
    node *stepexpr = FOR_STEP(arg_node) ? COPYdoCopy(FOR_STEP(arg_node)) : TBmakeNum(1);
    node *assignstep = TBmakeAssign(TBmakeVarlet(STRcpy(VARDECL_NAME(step)), step, NULL), stepexpr);
    node *assignstop = TBmakeAssign(TBmakeVarlet(STRcpy(VARDECL_NAME(stop)), stop, NULL), COPYdoCopy(FOR_STOP(arg_node)));
    node *assignstart = TBmakeAssign(TBmakeVarlet(STRcpy(VARDECL_NAME(start)), start, NULL), COPYdoCopy(FOR_START(arg_node)));

    node *stmtsstep = TBmakeStmts(assignstep, NULL);
    node *stmtsstop = TBmakeStmts(assignstop, stmtsstep);
    node *stmtsstart = TBmakeStmts(assignstart, stmtsstop);

    // remember the statments
    INFO_STATEMENTS(arg_info) = stmtsstart;

    // copy the blocks
    node *block = COPYdoCopy(FOR_BLOCK(arg_node));

    // create the assignemnt statement
    node *assign = TBmakeAssign(TBmakeVarlet(STRcpy(VARDECL_NAME(start)), start, NULL), TBmakeBinop(BO_add, TBmakeVar(STRcpy(VARDECL_NAME(start)), start, NULL), TBmakeVar(STRcpy(VARDECL_NAME(step)), step, NULL)));

    //append the statement to the end
    if (block == NULL)
        block = TBmakeStmts(assign, NULL);

    else
        append(block, TBmakeStmts(assign, NULL));

    // remove the node
    FREEdoFreeTree(arg_node);

    // create the conditions
    node *while_expr = TBmakeBinop(BO_lt, TBmakeVar(STRcpy(VARDECL_NAME(start)), NULL, NULL), TBmakeVar(STRcpy(VARDECL_NAME(stop)), NULL, NULL));
    DBUG_RETURN(TBmakeWhile(while_expr, block));
}

node *NFLvarlet(node *arg_node, info *arg_info)
{
    DBUG_ENTER("NFLvarlet");

    kvlistnode *node = KVLLsearch(INFO_INDUCTION_VARIABLES(arg_info), VARLET_NAME(arg_node));

    if (node)
    {
        VARLET_NAME(arg_node) = STRcpy(node->value);
    }

    DBUG_RETURN(arg_node);
}

node *NFLvar(node *arg_node, info *arg_info)
{
    DBUG_ENTER("NFLvar");

    kvlistnode *node = KVLLsearch(INFO_INDUCTION_VARIABLES(arg_info), VAR_NAME(arg_node));

    if (node)
    {
        VAR_NAME(arg_node) = STRcpy(node->value);
    }

    DBUG_RETURN(arg_node);
}

node *NFLdoNormalizeForLoops(node *syntaxtree)
{
    DBUG_ENTER("NFLdoNormalizeForLoops");

    info *info = MakeInfo();

    TRAVpush(TR_nfl);
    syntaxtree = TRAVdo(syntaxtree, info);
    TRAVpop();

    info = FreeInfo(info);

    DBUG_RETURN(syntaxtree);
}
