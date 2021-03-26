#include "normalize_for_loops.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "memory.h"
#include "free.h"
#include "str.h"
#include "copy.h"
#include "ctinfo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct list_node
{
    const char *old_name;
    const char *new_name;
    struct list_node *next;
} list_node;

list_node *IVLcreate(const char *old_name, const char *new_name, list_node *next)
{
    list_node *new_list_node = (list_node *)malloc(sizeof(list_node));
    new_list_node->old_name = old_name;
    new_list_node->new_name = new_name;
    new_list_node->next = next;

    return new_list_node;
}

list_node *IVLadd(list_node *list, const char *old_name, const char *new_name)
{
    if (list == NULL)
    {
        list = IVLcreate(old_name, new_name, NULL);
        return list;
    }

    list_node *cursor = list;
    while (cursor->next != NULL)
    {
        cursor = cursor->next;
    }

    list_node *new_kvlistnode = IVLcreate(old_name, new_name, NULL);
    cursor->next = new_kvlistnode;

    return list;
}

list_node *IVLfind(list_node *list, const char *old_name)
{

    list_node *cursor = list;
    while (cursor != NULL)
    {
        if (strcmp(cursor->old_name, old_name) == 0)
        {
            return cursor;
        }

        cursor = cursor->next;
    }

    return NULL;
}

void IVLdispose(list_node *head)
{
    list_node *cursor, *tmp;

    if (head != NULL)
    {
        cursor = head->next;
        head->next = NULL;
        while (cursor != NULL)
        {
            tmp = cursor->next;
            free(cursor);
            cursor = tmp;
        }
    }
}

struct INFO
{
    unsigned int for_loop_counter;
    list_node *induction_variables;
    
    node *variable_declarations;
    node *statements;
};

#define INFO_FOR_LOOP_COUNTER(n) ((n)->for_loop_counter)
#define INFO_INDUCTION_VARIABLES(n) ((n)->induction_variables)

#define INFO_VARDECLS(n) ((n)->variable_declarations)
#define INFO_STATEMENTS(n) ((n)->statements)

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

    IVLdispose(INFO_INDUCTION_VARIABLES(info));

    info = MEMfree(info);

    DBUG_RETURN(info);
}

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

node *NFLfunbody(node *arg_node, info *arg_info)
{
    DBUG_ENTER("NFLfunbody");

    info *funbody_info = MakeInfo();

    FUNBODY_STMTS(arg_node) = TRAVopt(FUNBODY_STMTS(arg_node), funbody_info);

    if (INFO_VARDECLS(funbody_info))
    {
        if (!FUNBODY_VARDECLS(arg_node))
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

    nodetype type = NODE_TYPE(STMTS_STMT(arg_node));

    STMTS_STMT(arg_node) = TRAVdo(STMTS_STMT(arg_node), arg_info);

    if (type == N_for)
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

    char *name = STRcatn(4, "_for_", STRitoa(INFO_FOR_LOOP_COUNTER(arg_info)), "_", FOR_LOOPVAR(arg_node));
    INFO_FOR_LOOP_COUNTER(arg_info)++;

    INFO_INDUCTION_VARIABLES(arg_info) = IVLadd(INFO_INDUCTION_VARIABLES(arg_info), FOR_LOOPVAR(arg_node), name);

    FOR_BLOCK(arg_node) = TRAVopt(FOR_BLOCK(arg_node), arg_info);

    node *vardecl_step = TBmakeVardecl(STRcat(name, "_step"), T_int, NULL, NULL, NULL);
    node *vardecl_stop = TBmakeVardecl(STRcat(name, "_stop"), T_int, NULL, NULL, vardecl_step);
    node *vardecl_start = TBmakeVardecl(STRcpy(name), T_int, NULL, NULL, vardecl_stop);

    if (INFO_VARDECLS(arg_info) == NULL)
    {
        INFO_VARDECLS(arg_info) = vardecl_start;
    }
    else
    {
        append(INFO_VARDECLS(arg_info), vardecl_start);
    }

    node *stepexpr = FOR_STEP(arg_node) ? COPYdoCopy(FOR_STEP(arg_node)) : TBmakeNum(1);
    node *assignstep = TBmakeAssign(TBmakeVarlet(STRcpy(VARDECL_NAME(vardecl_step)), vardecl_step, NULL), stepexpr);
    node *assignstop = TBmakeAssign(TBmakeVarlet(STRcpy(VARDECL_NAME(vardecl_stop)), vardecl_stop, NULL), COPYdoCopy(FOR_STOP(arg_node)));
    node *assignstart = TBmakeAssign(TBmakeVarlet(STRcpy(VARDECL_NAME(vardecl_start)), vardecl_start, NULL), COPYdoCopy(FOR_START(arg_node)));

    node *stmtsstep = TBmakeStmts(assignstep, NULL);
    node *stmtsstop = TBmakeStmts(assignstop, stmtsstep);
    node *stmtsstart = TBmakeStmts(assignstart, stmtsstop);

    // remember the statments
    INFO_STATEMENTS(arg_info) = stmtsstart;

    // copy the blocks
    node *block = COPYdoCopy(FOR_BLOCK(arg_node));

    // create the assignemnt statement
    node *assign = TBmakeAssign(TBmakeVarlet(STRcpy(VARDECL_NAME(vardecl_start)), vardecl_start, NULL), TBmakeBinop(BO_add, TBmakeVar(STRcpy(VARDECL_NAME(vardecl_start)), vardecl_start, NULL), TBmakeVar(STRcpy(VARDECL_NAME(vardecl_step)), vardecl_step, NULL)));

    //append the statement to the end
    if (block == NULL)
        block = TBmakeStmts(assign, NULL);

    else
        append(block, TBmakeStmts(assign, NULL));

    // remove the node
    FREEdoFreeTree(arg_node);

    // create the conditions
    node *while_expr = TBmakeBinop(BO_lt, TBmakeVar(STRcpy(VARDECL_NAME(vardecl_start)), NULL, NULL), TBmakeVar(STRcpy(VARDECL_NAME(vardecl_stop)), NULL, NULL));
    DBUG_RETURN(TBmakeWhile(while_expr, block));
}

node *NFLvarlet(node *arg_node, info *arg_info)
{
    DBUG_ENTER("NFLvarlet");

    list_node *node = IVLfind(INFO_INDUCTION_VARIABLES(arg_info), VARLET_NAME(arg_node));

    if (node)
    {
        VARLET_NAME(arg_node) = STRcpy(node->new_name);
    }

    DBUG_RETURN(arg_node);
}

node *NFLvar(node *arg_node, info *arg_info)
{
    DBUG_ENTER("NFLvar");

    list_node *node = IVLfind(INFO_INDUCTION_VARIABLES(arg_info), VAR_NAME(arg_node));

    if (node)
    {
        VAR_NAME(arg_node) = STRcpy(node->new_name);
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
